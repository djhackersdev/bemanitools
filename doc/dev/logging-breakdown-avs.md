Copy/pasted from chat with tau (2018/02/10):

alright then. so for modern iidx.

we want to do logging inside iidxhook and we also want to pass AVS-style log functions to iidxio which in turn passes them on to geninput in order for those to do logging too

so iidxhook connects to the AVS log API: log_body_misc and friends, which I assume are invoked using a log_misc() macro in Konami's source code that adds some sort of module tag.

anyway yeah this we already know.

libutil has four function ptrs: log_impl_misc and co. These are static variables which are statically initialized to some no-op functions. Except log_impl_fatal, whose implementation just calls libc abort()

at startup you call log_to_external(), supplying four function ptrs to wire these up to. As the name suggests, this causes Bemanitools libutil to talk to something that is compatible with the AVS log sink API.

alternatively you can log_to_writer(), which initializes Bemanitools to use its own, internal logging system, and you give it a log writer function that takes strings and writes them somewhere.

So you have log sinks and you have log writers. The path is [application code] -> [log sink] -> [logging engine] -> [log writer]
19:29

inside config.exe (or generally outside of modern AVS games) this path looks like [bemanitools application code] -> [log sinks passed across dlls] -> [bemanitools logging engine] -> [bemanitools log writer]

inside modern AVS game the path looks like [bt hook dll / bt iodev dll] -> [avs log_body_whatever log sinks] -> [avs logging engine] -> [launcher.exe log writer]

note that I tried to keep the log writer API consistent with the AVS log writer API but then Konami went and broke it repeatedly so now Bemanitools has its own stable log writer API. Launcher tracks the AVS log writer API, which breaks constantly, so that's not the same thing.

anyway that's the background story. Now for the details about IIDX in particular.

up until about iidx19 we did things the obvious way: iidxhook would log_to_external() to hook into the AVS log sinks and then call those directly and all was well. Then one fine day I was given a IIDX19 data dump and tried running iidxhook and it crashed with a stack overflow. hmm.

the problem boils down to this: iidx19 AVS added those log timestamps. And for for whatever reason the AVS logging engine needs to access some mutexes and condition variables to make this work properly

but AVS of course in grand Konami tradition has its own threading and concurrency primitive API which wraps the Win32 API. tbf this is kind of understandable in some sense, because win32 actually did not have condition variables until Windows Vista! in 2006! seriously, I'm not kidding.

there's all sorts of articles out there describing in fine detail how to use Win32's event objects to implement your own condition variables and the multitudinous pitfalls that this entails

but anyway one fun thing about the AVS concurrency API is that you can't actually use concurrency primitives unless you're calling that API from a thread launched using the AVS threading API

and that's a problem in the case of iidxhook, because iidx is old as balls relatively speaking and its EZUSB driver code has I think two worker threads, which it launches using the MS libc's _beginthreadex() function

this in turn is a wrapper around the win32 CreateThread function, but it also boots up stdio on whatever new thread gets launched and basically is responsible for guaranteeing that the libc will operate correctly on the newly launched thread

so yeah when you do windows programming, never call CreateThread, always call _beginthreadex. otherwise stuff will break. maybe.

point is, IIDX predates modern AVS so it just uses Windows threading directly. So, IIDX worker thread starts up, iidxhook does its thing, writes a log message, calls into AVS logging, which in turn grabs an AVS mutex, the implementation of which says "omg this isn't an AVS thread aaaaaa" and ... attempts to call back into the logging system to log this fact. whereupon a stack overflow condition proceeds in a predictable manner.

so, there are a few ways to deal with this problem. bemanitools 4 dealt with it in a fairly stupid way.

and very elaborate way too

bt4 intercepted IIDX's calls to create Windows threads and then redirected those calls to go via the AVS threading API

so now the worker threads are AVS threads and logging works as expected

which is all well and good but the problem is that these threads are quite timing critical. it probably worked fine, but i didn't want to risk affecting those threads in a weird way and introducing latency and jitter. i wanted to keep the threading pristine and not mess with it just for the sake of diagnostic messages

so bemanitools 5 uses the log server approach

at an appropriate time, it creates its own AVS thread, the logging server. Since it is an AVS thread, it can call the AVS logging API

and then we have log_post_misc() and friends, implemented in log-server.c

we initialize the Bemanitools logging system to log "externally" to those funcs and we also propagate those to iidxio.dll and eamio.dll which in turn pass them to geninput.dll

so what do log_post_misc and friends do

they lock a "mailbox" using the win32 concurrency primitives and write the log severity and a pointer to the string to be logged into the mailbox, then signal the log server thread, again using win32 concurrency primitives

then they do a synchronous wait for an acknowledgement from the logging server: since we're holding a string pointer, we cannot return until that string pointer has been consumed or it may be concurrently invalidated

so the log server wakes up, locks the mailbox, calls AVS log_body_misc() to write the log message, then once that returns it asserts a signal in the mailbox (again, win32 event object because lol what are condition variables) and releases the lock.

the caller gets the signal, wakes up, and returns to whatever Bemanitools code is running on the IIDX IO worker thread that wanted to write a log message.

end of essay.


