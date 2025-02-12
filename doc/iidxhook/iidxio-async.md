# IIDXIO async API implementation

This implementation of the iidxio API is a shim library that takes another iidxio library and
runs the functions `iidx_io_ep1_send`, `iidx_io_ep2_recv` and `iidx_io_ep3_write_16seg` in a
dedicated thread. State synchronization to the getter and setter functions is also handled
transparently.

Usage of this **may** improve performance of certain iidxio implementations or when using them
in certain integrations, e.g. the send and receive functions of a iidxio implementation for some
target IO hardware calls are synchronous and expensive. 

This is not a fix/solution to a badly implemented iidxio library with poor performance as it cannot
make it go faster and address potential latency issues, for example.

Use with caution and know why and when you need to use it.

## Setup

* Add `iidxio-async.dll` in the same folder as your `iidxhookX.dll`
* Rename your `iidxio.dll` to `iidxio-async-child.dll`
* Rename `iidxio-async.dll` to `iidxio.dll`
* Run the game