#ifndef FSK_H
#define FSK_H

#include <stdint.h>
#include <stdbool.h>

void FSKconfiguration(void);
float FSKdemodulator(uint16_t data);

#endif //FSK_H
