# Contributing

This document outlines different types of contributions and how YOU can help us to improve the
project. Read it, as it provides guidelines that are there to help you and the maintainers.

## Reporting and discussions: Issues section on gitlab

In order to avoid having to manage ongoing discussions and bug reports on different communication
channels, e.g. forums, messangers or other closed groups, we ask everyone to treat the issue section
on gitlab as the place to open their relevant discussions and bug reports regarding the project.

The maintainers of the project do not have the time nor motivation to micromanage on the various
channels and enter all data here to have it collected. This is a simple task that *ANYONE* can do
allowing the maintainers and developers to spend their time on the codebase.

## Bug reports

Follow these steps when reporting bugs to ensure you provide all information we *always* need and
make your report valuable and actionable.

1. On the bemanitools repository, go `Issues` on the left-hand sidebar.
1. Use the search function to check if there is an already open issue regarding what you want to
   report
   1. If that applies, read the open issue to check what's already covered regarding the bug
   1. Provide additional information or things that are missing. Upload your log files, screenshots,
      videos etc. Be careful to remove sensitive information like PCBIDs
   1. Give a thumbs up to the issue to show you are interested/affected as well
1. If no existing issue avilable, create a new one
1. Come up with a descriptive title
1. **USE OUR BUG REPORTING TEMPLATE**: Pick it by selecting `Bug` on the `Description` section
1. Follow the sections and their instructions provided by the template and fill them in. All fields
   are mandatory to provide a comprehensive report if not stated otherwise
1. When finished, submit the issue

## Pull requests: bugfixes, new features or other code contributions

Pull requests are welcome! May it be a merge request to an already known issue or a new feature that
you consider as a valuable contribution, please open a MR.

**!!! Maintaining documentation by adding new or improving existing documentation is as important as
code !!!**

If you want to start working on a new feature that was proposed in an issue, yet, it is recommended
to reach out to the active developers about this, first, to discuss if this contribution is valuable
to the project. Otherwise, you might waste your time on implementing something that won't make it
into master.

Please read our [development guidelines](doc/development.md) as they contain valuable information
that your contribution meets our standards. This is not meant to annoy people but ensures
consistency that the project stays maintainable for everyone.

Steps for contributing to the repository using a merge request:

1. If you are new to git, take a bit of time to learn the basics which are very simple, e.g. Google
   for "git tutorial for non-programmers"
1. Fork the upstream repository (Fork button on the top right on the main page of the repository)
1. You can start editing files like documentation easily inside gitlab which might be the prefered
   option for many non-coders
1. Clone your fork to your local machine and start working on stuff
1. Ensure you push your changes to your fork on gitlab
1. When done, go to the `Merge Requests` section on the left sidebar of the upstream repository
1. Hit the `New merge request` button
1. Select the `master` branch as the source branch
1. Select whatever branch you worked, likely `master` if you didn't change that, as the target
   branch
1. Hit `Compare branches and continue`
1. Provide a descriptive title of what your change is about
1. **USE OUR MR TEMPLATES**
   1. If you submit a bugfix, use the `Bugfix` tempalte and fill in the sections
   1. If you submit a new feature, use the `Feature` template and fill in the sections
1. If you submit some minor fixes or documentation improvements, there is no template for that.
   Please provide a expressive description what you did and *why* you did that
1. If any of your changes are tied to one or multiple issues, link them in the description
1. When done, hit `Submit merge request`

The maintainers will take a look at your submission and provide their feedback. The intention of
this process is to ensure the contribution meets the quality standards. Please also see this is a
learning opportunity, especially with your first contribution, if a lot of comments and change
requests are being made. The maintainers are open to discuss their suggestions/feedback if
reasonable feedback is given back to them.

Once all discussion is resolved and the involved maintainers approved your submission, it will be
merged into master and also included in the next release.

## Roadmap

No concrete roadmap or timeline exists. We want to continue adding support for new games as well as
old games (some of the old games supported by BT4 are not supported, yet).
