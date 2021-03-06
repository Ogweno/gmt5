.. index:: ! psvelo

******
psvelo
******

.. only:: not man

    psvelo - Plot velocity vectors, crosses, and wedges on maps

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**psvelo** [ *table* ] **-J**\ *parameters*
|SYN_OPT-R|
[ **-A**\ *parameters* ]
[ |SYN_OPT-B| ]
[ **-F**\ *color* ] [ **-E**\ *color* ] [ **-G**\ *fill* ] [ **-K** ]
[ **-L** ] [ **-N** ] [ **-O** ] [ **-P** ]
[ **-S**\ *symbol*/*scale*/*conf*/*font\_size* ] [
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ **-W**\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-c| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-:| ]

|No-spaces|

Description
-----------

**psvelo** reads data values from *files* [or standard input] and
generates PostScript code that will plot velocity arrows on a map.
Most options are the same as for :doc:`psxy </psxy>`, except **-S**. The
PostScript code is written to standard output. The previous version
(**psvelomeca**) is now obsolete. It has been replaced by **psvelo** and
:doc:`psmeca`.

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_intables.rst_

.. include:: ../../explain_-J.rst_

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rgeo.rst_

**-S**

Selects the meaning of the columns in the data file and the figure to be
plotted.

**-Se**\ *velscale/confidence/fontsize*.

Velocity ellipses in (N,E) convention. *Vscale* sets the scaling of the
velocity arrows. This scaling gives inches (unless **c**, **i**, **m**,
or **p** is appended). *Confidence* sets the 2-dimensional confidence
limit for the ellipse, e.g., 0.95 for 95% confidence ellipse. *Fontsize*
sets the size of the text in points. The ellipse will be filled with the
color or shade specified by the **-G** option [default transparent]. The
arrow and the circumference of the ellipse will be drawn with the pen
attributes specified by the **-W** option. Parameters are expected to be
in the following columns:

    **1**,\ **2**:
    longitude, latitude of station (**-:** option interchanges order)
    **3**,\ **4**:
    eastward, northward velocity (**-:** option interchanges order)
    **5**,\ **6**:
    uncertainty of eastward, northward velocities (1-sigma) (**-:**
    option interchanges order)
    **7**:
    correlation between eastward and northward components
    **8**:
    name of station (optional).

**-Sn**\ *barscale.*

Anisotropy bars. *Barscale* sets the scaling of the bars This scaling
gives inches (unless **c**, **i**, **m**, or **p** is appended).
Parameters are expected to be in the following columns:

    **1**,\ **2**:
    longitude, latitude of station (**-:** option interchanges order)
    **3**,\ **4**:
    eastward, northward components of anisotropy vector (**-:** option
    interchanges order)

**-Sr**\ *velscale/confidence/fontsize*

Velocity ellipses in rotated convention. *Vscale* sets the scaling of
the velocity arrows. This scaling gives inches (unless **c**, **i**,
**m**, or **p** is appended). *Confidence* sets the 2-dimensional
confidence limit for the ellipse, e.g., 0.95 for 95% confidence ellipse.
*Fontsize* sets the size of the text in points. The ellipse will be
filled with the color or shade specified by the **-G** option [default
transparent]. The arrow and the circumference of the ellipse will be
drawn with the pen attributes specified by the **-W** option. Parameters
are expected to be in the following columns:

    **1**,\ **2**:
    longitude, latitude, of station (**-:** option interchanges order)
    **3**,\ **4**:
    eastward, northward velocity (**-:** option interchanges order)
    **5**,\ **6**:
    semi-major, semi-minor axes
    **7**:
    counter-clockwise angle, in degrees, from horizontal axis to major
    axis of ellipse.
    **8**:
    name of station (optional)

**-Sw**\ *wedge\_scale/wedge\_mag*.

Rotational wedges. *Wedge\_scale* sets the size of the wedges in inches
(unless **c**, **i**, **m**, or **p** is appended). Values are
multiplied by *Wedge\_mag* before plotting. For example, setting
*Wedge\_mag* to 1.e7 works well for rotations of the order of 100
nanoradians/yr. Use **-G** to set the fill color or shade for the wedge,
and **-E** to set the color or shade for the uncertainty. Parameters are
expected to be in the following columns:

    **1**,\ **2**:
    longitude, latitude, of station (**-:** option interchanges order)
    **3**:
    rotation in radians
    **4**:
    rotation uncertainty in radians

**-Sx**\ *cross\_scale*

gives Strain crosses. *Cross\_scale* sets the size of the cross in
inches (unless **c**, **i**, **m**, or **p** is appended). Parameters
are expected to be in the following columns:

    **1**,\ **2**:
    longitude, latitude, of station (**-:** option interchanges order)
    **3**:
    eps1, the most extensional eigenvalue of strain tensor, with
    extension taken positive.
    **4**:
    eps2, the most compressional eigenvalue of strain tensor, with
    extension taken positive.
    **5**:
    azimuth of eps2 in degrees CW from North.

Optional Arguments
------------------

**-A**\ *parameters*
    Modify vector parameters. For vector heads, append vector head
    *size* [Default is 9p]. See VECTOR ATTRIBUTES for specifying
    additional attributes.

.. include:: ../../explain_-B.rst_

**-D**\ *Sigma\_scale*
    can be used to rescale the uncertainties of velocities (**-Se** and
    **-Sr**) and rotations (**-Sw**). Can be combined with the *confidence* variable.

**-F**\ *fill*
    Sets the color or shade used for frame and annotation. [Default is black]

**-E**\ *fill*
    Sets the color or shade used for filling uncertainty wedges
    (**-Sw**) or velocity error ellipses (**-Se** or **-Sr**). [If
    **-E** is not specified, the uncertainty regions will be transparent.]

**-G**\ *fill*
    Specify color (for symbols/polygons) or pattern (for polygons)
    [Default is black]. Optionally, specify
    **-Gp**\ *icon\_size/pattern*, where *pattern* gives the number of
    the image pattern (1-90) OR the name of a icon-format file.
    *icon\_size* sets the unit size in inches. To invert black and white
    pixels, use **-GP** instead of **-Gp**. See **pspatterns** for
    information on individual patterns.

.. include:: ../../explain_-K.rst_

**-L**
    Draw lines. Ellipses and fault planes will have their outlines drawn
    using current pen (see **-W**).
**-N**
    Do **NOT** skip symbols that fall outside the frame boundary
    specified by **-R**. [Default plots symbols inside frame only].

.. include:: ../../explain_-O.rst_
.. include:: ../../explain_-P.rst_
.. include:: ../../explain_-U.rst_

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

**-W**
    Set pen attributes for velocity arrows, ellipse circumference and
    fault plane edges. [Defaults: width = default, color = black, style = solid].

.. include:: ../../explain_-XY.rst_
.. include:: ../../explain_-c.rst_
.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_

.. include:: ../../explain_-icols.rst_
.. include:: ../../explain_colon.rst_
.. include:: ../../explain_help.rst_

.. include:: ../../explain_vectors.rst_

Examples
--------

The following should make big red arrows with green ellipses, outlined
in red. Note that the 39% confidence scaling will give an ellipse which
fits inside a rectangle of dimension Esig by Nsig.

   ::

    gmt psvelo << END -h2 -R-10/10/-10/10 -W0.25p,red -Ggreen -L -Se0.2/0.39/18 \
        -B1g1 -Jx0.4/0.4 -A0.3p -P -V > test.ps
    #Long. Lat. Evel Nvel Esig Nsig CorEN SITE
    #(deg) (deg) (mm/yr) (mm/yr)
    0. -8. 0.0 0.0 4.0 6.0 0.500 4x6
    -8. 5. 3.0 3.0 0.0 0.0 0.500 3x3
    0. 0. 4.0 6.0 4.0 6.0 0.500
    -5. -5. 6.0 4.0 6.0 4.0 0.500 6x4
    5. 0. -6.0 4.0 6.0 4.0 -0.500 -6x4
    0. -5. 6.0 -4.0 6.0 4.0 -0.500 6x-4
    END

This example should plot some residual rates of rotation in the Western
Transverse Ranges, California. The wedges will be dark gray, with light
gray wedges to represent the 2-sigma uncertainties.

   ::

    gmt psvelo << END -Sw0.4/1.e7 -W0.75p -Gdarkgray -Elightgray -h1 -D2 -Jm2.2 \
        -R240./243./32.5/34.75 -Bf10ma60m/WeSn -P > test.ps
    #lon lat spin(rad/yr) spin_sigma (rad/yr)
    241.4806 34.2073 5.65E-08 1.17E-08
    241.6024 34.4468 -4.85E-08 1.85E-08
    241.0952 34.4079 4.46E-09 3.07E-08
    241.2542 34.2581 1.28E-07 1.59E-08
    242.0593 34.0773 -6.62E-08 1.74E-08
    241.0553 34.5369 -2.38E-07 4.27E-08
    241.1993 33.1894 -2.99E-10 7.64E-09
    241.1084 34.2565 2.17E-08 3.53E-08
    END

See Also
--------

:doc:`gmt </gmt>`, :doc:`psbasemap </psbasemap>`, :doc:`psxy </psxy>`

References
----------

Bomford, G., Geodesy, 4th ed., Oxford University Press, 1980.

Authors
-------

`Kurt L. Feigl <http://www.geology.wisc.edu/~feigl/>`_, Department of Geology and
Geophysics at University of Wisconsin-Madison, Madison, Wisconsin, USA

Genevieve Patau, `Laboratory of Seismogenesis <http://www.ipgp.fr/rech/sismogenese/>`_,
Institut de Physique du Globe de Paris, Departement de Sismologie, Paris, France
