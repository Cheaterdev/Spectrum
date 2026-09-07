// Only GraphicsPSO VoxelDebug (voxel.sig) still compiles this file, for its
// generic fullscreen-quad VS -- the old voxel-cone-traced GI pipeline that
// used to live here (VoxelScreen's CS/PS/PS_Resize entries: getGI/trace/
// get_history and friends) is gone now that NRD REBLUR_DIFFUSE/SPECULAR
// fully replaces it (see [[project-nrd-integration]]).
#include "../2D_screen_simple.h"
