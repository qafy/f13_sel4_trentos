/*
 * Copyright (C) 2023, Jakob Kukla
 */

#ifndef _RPi_SPI_TPM_H_
#define _RPi_SPI_TPM_H_

WOLFTPM2_DEV dev;

OS_Error_t tpm_int_clear_tpm();

OS_Error_t tpm_int_get_or_create_srk(WOLFTPM2_KEY *srk);

#endif /* _RPi_SPI_TPM_H_ */