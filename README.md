# Smart Frame

Smart frame is a project I'm working on to display to an ePaper display using 
a Raspberry Pi Zero W.

The project relies on the `bcm2835` library which can be found in the vendor
folder. The library must be installed using `cd vendor/bcm2835 | make install`.

Another library that is going to be used is `libjpeg` more will be written about
this when I actually implement it.



## Running

```bash
make
sudo ./smartframe.out
```

