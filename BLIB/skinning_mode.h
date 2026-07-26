#pragma once

// This is the toggle for using either CPU or GPU skinning. 

// To enable CPU skinning, uncomment the below definition. To enable GPU skinning, comment out the below definition.
//#define SKIN_CPU

#ifdef SKIN_CPU
#undef SKIN_GPU
#else
#define SKIN_GPU
#endif // SKIN_CPU