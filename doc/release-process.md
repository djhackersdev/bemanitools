# Release process

For maintainers, steps for the release process:

1. Update the corresponding section in the [changelog](CHANGELOG.md) with bullet points reflecting
   major features merged into master since the previous release

1. Make sure you have the latest `master` state pulled to your local copy

1. Tag the release with the next release number which also be found at the
   [top of the readme](../README.md) as it always reflects the current version being worked on on
   the `master` branch , e.g. 0.04: `git tag 5.35`

1. Push the tag to upstream: `git push origin 5.35`

1. The [build pipeline](https://github.com/djhackersdev/bemanitools/actions) should start
   automatically once the tag is pushed including the steps `build` and `publish-release`

1. Once completed successfully, the release is published on the
   [releases page](https://github.com/djhackersdev/bemanitools/releases/)

1. Edit the latest release

1. Copy-pate the change log section of the respective version into the description

1. Un-check *Set as pre-release*

1. Check *Set as the latest release*

1. Click *Update release*

1. Notify any other channels, e.g. the pigs in the stall, about the latest release:

   1. New post in thread
      ```
      <insert version here> released: <direct link to published version>

      Changelog:
      <paste changelog here>
      ```
   1. Update the OP title by bumping the version number in it
   1. Update the OP post by extending it accordingly (see previous entries)

1. Bump the version number at the [top of the readme](../README.md) and add a new empty section in
   the [changelog](../CHANGELOG.md) with the new version number as title. Commit changes and push to
   `master` branch

1. Continue developing and merging PRs until you decide its time for another release
