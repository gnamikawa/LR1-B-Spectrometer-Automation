
#include <stdio.h>
void smpl_ReadAndWriteToDevice(unsigned char	*InputReport1, unsigned char	*OutputReport1, int	DevDet);
void smpl_ReadAndWriteToDevice_new(unsigned char	*InputReport1, unsigned char	*OutputReport1, int	DevDet);
int smpl_GetSpectra(signed short	*InputSpec1, unsigned char SpecNmb, unsigned short startPix, unsigned short endPix, unsigned char Fast, unsigned char test1, unsigned short tot_startPix, unsigned short tot_endPix);
bool smpl_DevDetect();
bool smpl_FindTheHID();
void smpl_reset();
void smpl_resetAddress();
void smpl_shutdown();
