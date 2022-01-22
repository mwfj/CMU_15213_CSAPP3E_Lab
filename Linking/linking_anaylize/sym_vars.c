// Object(bind, type, section index)

// For the object COMMON section, the compiler cannot decide whether `var0` is initialized or not
// Cause `var0 are defined in the another place
// Thus, var0 may direct to .data or .bss, where it depends on the value that assigns to
int var0; // (Global, object, COMMON)
int var1 = 0; // (Global, object, .bss)
int var2 = 1; // (Global, object, .data)

extern int var3; // (Global, object, UNDEF)
// warning here equal to remove external
extern int var4 = 0; // (Global, object, .bss)
// warning here equal to remove external 
extern int var5 = 1; // (Global, object, .data)

// compiler will assign it to 0 just like var7
static int var6;   // (Local, object, .bss)
static int var7 = 0; // (Local, object, .bss)
static int var8 = 1; // (Local, object, .data)

__attribute((weak)) int var9; // (Weak, object, .bss)
__attribute((weak)) int vara = 0; // (Weak, object, .bss)
__attribute((weak)) int varb = 1; // (Weak, object, .text)


// // (Global, func, .data)
void foo(){
  var0 = 2;  
  var1 = 2;  
  var2 = 2;  
  var3 = 2;  
  var4 = 2;  
  var5 = 2;  
  var6 = 2;  
  var7 = 2;  
  var8 = 2;  
  var9 = 2;  
  vara = 2;  
  varb = 2;  
}
