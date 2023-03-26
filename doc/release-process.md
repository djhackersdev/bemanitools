# Release process

For bemanitools maintainers, steps for the release process:

1. Update the corresponding section in the [changelog](CHANGELOG.md) with bullet points reflecting
major features merged into master since the previous release
1. Make sure you have the latest `master` state pulled to your local copy
1. Tag the release with the next release number which also be found at the
[top of the readme](#bemanitools-5) as it always reflects the current version beinged worked on
on the `master` branch , e.g. 5.35: `git tag 5.35`
1. Push the tag to upstream: `git push origin 5.35`
1. The [build pipeline](https://dev.s-ul.net/djhackers/bemanitools/-/pipelines) should start
automatically once the tag is pushed including the steps `build` and `upload-release`
1. Once completed successfully, the release is uploaded
1. Take the changelog of the published version and notify the pigs in the stall about it:
    1. New post in thread
        ```
        <insert version here> released: <direct link to published version>

        Changelog:
        <paste changelog here>
        ```
    1. Update the OP title by bumping the version number in it
    1. Update the OP post by extending it accordingly (see previous entries)

1. Bump the version number at the [top of this readme](#bemanitools-5) and add a new empty section
in the [changelog](CHANGELOG.md) with the new version number as title. Commit changes and push to
`master` branch
1. Continue developing and merging MRs until you decide its time for another release