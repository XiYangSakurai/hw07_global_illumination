Path Tracer Episode V: Global Illumination Strikes Back
======================

**University of Pennsylvania, CIS 561: Advanced Computer Graphics, Homework 7**

======================

* Name: Xi Yang
* PennKey: 43486159

======================
Extra Credit:
------------
MicrofacetBTDF

I also wrote two material classes : MicrofacetTransmissiveMaterial and MicrofacetMaterial.
100 sample, 5 limit

MicrofacetTransmissiveMaterial: PT_MicrofacetTransmitBox.json

![](./02.png)

MicrofacetMaterial: PT_MicrofacetGlassBallBox.json

![](./03.png)


Results:
------------

twolightsCornell
------------
100 sample, 5 limit

![](./01.png)

Custom Scenes:
------------
All rendered with 400 sample, 8 limit

This scene contains three different kinds of transmissive materials, two specular(yellow sphere and small cube), one microfacet(bigger cube and green sphere).
(scene1.jason)

![](./glass400.png)

This scene has chrome material and transmissive materials. But there are so many fireflies.
(scene2.jason)

![](./uniform400.png)

This scene has different transmissive materials and matteMaterials.
(scene3.jason)

![](./scene3.png)

This scene has a mesh obj and several materials(glass,microfacet glass, etc).
(scene4.jason)

![](./AAscene.png)

This scene has a mirror, one glass ball and one mesh.
(scene5.jason)

![](./ball900-8.png)
