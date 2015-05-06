The following files are simply duplicates from the Bootstrap library. With
modifications in were to reference the imports. In theory, you could replace
them if you update the Bootstrap framework by copying them from that framework
again. Just make sure to modify the import paths to reference the
`./bootstrap/less` folder:
```
./less/bootstrap.less
./less/responsive.less
./less/variables.less
```

The `./less/variables.less` file is generally where you will spend most of
your time customizing the various Bootstrap settings. Feel free to manually
edit it or even replace it with a service like
[BootTheme](http://www.boottheme.com).

The `./less/fixes.less` file contains various Bootstrap and Drupal fixes. It
may contain a few enhancements, feel free to edit this file as you see fit.

The following files are relatively blank (they may contain some code for the
inital sub-theme), but this is where you will actually spend most of your time
specifying specific styling for your sites configuration.
```
./less/header.less
./less/content.less
./less/footer.less
```

And finally, the `./less/style.less` file is the glue that holds it all
together and compiles everything into one file. Generally, you will not need
to modify this file unless you need to add or remove imported files. For
example, if you do not want your site to have responsive capabilities, free to
comment or remove that line. If you are a file hierarchy wizard and need to
separate your theme into multiple files, insert additional `@import '...';`
lines. 
