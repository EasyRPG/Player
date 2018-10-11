Put a textfile corresponding to the version number of the APK here and it will be
used as Changelog entry for that APK.

For example:

`android:versionCode` property in `AndroidManifest.xml` is "1234", then use
`1234.txt`.

You can use the `changelog.sh` script to generate this easily (assuming your
working directory is the `android` parent folder):

    ./changelog.sh > metadata/en-US/changelogs/1234.txt
