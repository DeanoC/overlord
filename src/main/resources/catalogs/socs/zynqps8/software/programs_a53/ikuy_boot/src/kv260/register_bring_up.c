//
// Created by deano on 2/1/22.
//

extern unsigned long psu_mio_init_data(void);
extern unsigned long psu_peripherals_pre_init_data(void);
extern unsigned long psu_pll_init_data(void);
extern unsigned long psu_clock_init_data(void);
extern unsigned long psu_ddr_init_data(void);
extern unsigned long psu_ddr_phybringup_data(void);
extern unsigned long psu_peripherals_init_data(void);
extern int init_serdes(void);
extern int init_peripheral(void);
extern unsigned long psu_peripherals_powerdwn_data(void);
extern unsigned long psu_afi_config(void);
extern unsigned long psu_ddr_qos_init_data(void);

extern void mioRunInitProgram();
extern void pllRunInitProgram();
extern void clockRunInitProgram();
extern void afiRunInitProgram();
extern void resetInRunInitProgram();
extern void serdesFixcal();
extern void serdesRunInitProgram();
extern void resetOutRunInitProgram();

void RegisterBringUp() {
	int status = 1;

//	status &= psu_mio_init_data();
	mioRunInitProgram();
	status &= psu_peripherals_pre_init_data();
	pllRunInitProgram();
	clockRunInitProgram();

	status &= psu_ddr_init_data();
	status &= psu_ddr_phybringup_data();
	status &= psu_peripherals_init_data();

	resetInRunInitProgram();

	serdesFixcal();

	serdesRunInitProgram();

	resetOutRunInitProgram();

	init_peripheral();

	afiRunInitProgram();

	psu_ddr_qos_init_data();
}
