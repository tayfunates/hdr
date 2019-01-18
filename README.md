# hdr
A couple of paper implementations in high dynamic range imaging using C++ 

1. (hdr-radiance-maps) Recovering High Dynamic Range Radiance Maps from Photographs. 
  * [Debevec et al](http://www.pauldebevec.com/Research/HDR/debevec-siggraph97.pdf).
    * StringUtil: Some string manupulation functions.
    * DebevecPixelSelection: Complex pixel selection method for running Debevec's algorithm.
    * EigenDebevecSolver: Jacobian SVD solver wrapper.
    * DebevecHDRRadianceMapsImpl: Method implementation file.
