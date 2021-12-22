# Linking 

Linking is the process of collecting and combing various pieces of code and data **into a single file** that can be loaded(copied) into memory and executed. `Linking can be performed at compile time`:

+ when the source code is translated into machine code; 
+ at load time, when the program loaded into memory and executed by the loader;
+ at runtime, by application programs

Linker plays a crucial role in software development because they enable ***separate compilation***. Instead of organizing a large application as one monolithic source file, we can **decompose it into smaller, more manageable modules** that can be modified and compiled separately. When we change one of these modules, we simply recomile it and relink the application, without having to recompile the other files.