Theses files contains the source code for the WarpGLOBE program. You will need the earth bitmap too, in order to make it work. You can find it in the original package of WarpGLOBE.

It is ready to be compiled with EMX (the OS/2 version of GCC).

GLOBE.C contains the source code for WarpGLOBE itself.
MAPA.C contains the source code of a plane generator, that shows the cities over a non-spheric map.
WARPERS.C contains the source code for the generator of the list of cities. If two cities have the same coordinates, it will stop with an error, indicating the position in the file and the coordinates, allowing to move them one or two pixels.

WarpGLOBE is distributed under the GPL license (see GPL.TXT for details), and was programed originally for OS/2 by Sergio Costas Rodriguez (Raster Software Vigo). You can contact with him in:

	scostas@arrakis.es

	http://raster.cibermillennium.com
