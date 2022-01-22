// fun(bind,  type, section index)

extern void f1();	// (Global, notype, UNDEF)
extern void f2(){}  // (Global, func, .text)

void f3(); // (Global, notype, UNDEF)
void f4(){} // (Global, func, .text)

__attribute__((weak)) extern void f5(); // (Weak, notype, UNDEF)
__attribute__((weak)) extern void f6(){} // (Weak, func, .text)

__attribute__((weak)) void f7(); // (Weak, notype, UNDEF)
__attribute__((weak)) void f8(){} // (Weak, func, .text)

// warning here equal to remove static
static void f9(); // (Global, notype, UNDEF)
static void fa(){} // (Local, func, .text)

// (Global, func, .text)
void g(){
	f1();
	f2();
	f3();
	f4();
	f5();
	f6();
	f7();
	f8();
	f9();
	fa();
}
