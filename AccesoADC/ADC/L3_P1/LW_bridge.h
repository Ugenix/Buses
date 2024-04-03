#ifndef LW_BRIDGE_H
#define LW_BRIDGE_H


// Prototypes for functions used to access physical memory addresses 
int open_physical (int);
void * map_physical (int, unsigned int, unsigned int);
void close_physical (int);
int unmap_physical (void *, unsigned int);

#endif //LW_BRIDGE_H
