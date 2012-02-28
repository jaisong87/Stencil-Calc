distributedStencil:
	cilk++ -m64 -fPIC -shared -o libstencil.so stencilUtils.cilk 
	mpicxx distributedMemStencil.cpp -Wl,-rpath=. -L. -lstencil -o distributedStencil
