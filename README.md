# hdr
A couple of paper implementations in high dynamic range imaging using C++ 

1. (hdr-radiance-maps) Recovering High Dynamic Range Radiance Maps from Photographs. 
  * [Debevec et al.](http://www.pauldebevec.com/Research/HDR/debevec-siggraph97.pdf)
    * StringUtil: Some string manupulation functions.
    * DebevecPixelSelection: Complex pixel selection method for running Debevec's algorithm.
    * EigenDebevecSolver: Jacobian SVD solver wrapper.
    * DebevecHDRRadianceMapsImpl: Method implementation file.

2. (gradient-domain-hdr-compression) Gradient Domain High Dynamic Range Compression.
  * [Fattal et al.](http://www.cs.huji.ac.il/~danix/hdr/hdrc.pdf)
    * StringUtil: Some string manupulation functions.
    * HDRLoader: Tool for load HDR image and convert to a set of float32 RGB triplet.
    * Laplace: Tool for solving the 2D Poisson equation: a1 u_xx + a2 u_yy = f
    * FattalToneMapper: Method implementation file
