//
// Created by deano on 2/1/22.
//

extern int psu_init(void);

extern void mioRunInitProgram(void);
extern void pllRunInitProgram(void);
extern void clockRunInitProgram(void);
extern void ddrRunInitProgram(void);
extern void peripheralsRunInitProgram(void);
extern void serdesRunInitProgram(void);
extern void miscRunInitProgram(void);
extern void ddrQosRunInitProgram(void);

void RegisterBringUp() {
#if defined(USE_XILINX_REGISTER_BRINGUP)
	psu_init();
#else
	// Register data is prefilled into the heap, so don't use
	// the heap until you have programmed the registers!
	mioRunInitProgram();
	pllRunInitProgram();

	clockRunInitProgram();
	peripheralsRunInitProgram();

	ddrRunInitProgram();
	serdesRunInitProgram();
	miscRunInitProgram();

	ddrQosRunInitProgram();
#endif
}
