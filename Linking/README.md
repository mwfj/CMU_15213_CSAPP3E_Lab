# Linking 

Linking is the process of collecting and combing various pieces of code and data **into a single file** that can be loaded(copied) into memory and executed. `Linking can be performed at compile time`:

+ when the source code is translated into machine code; 
+ at load time, when the program loaded into memory and executed by the loader;
+ at runtime, by application programs

Linker plays a crucial role in software development because they enable ***separate compilation***. Instead of organizing a large application as one monolithic source file, we can **decompose it into smaller, more manageable modules** that can be modified and compiled separately. When we change one of these modules, we simply recomile it and relink the application, without having to recompile the other files. 

 Most compilation systems provide a **compiler driver** that invokes:

```txt
Language Preprocessor -> Compiler -> Assembler -> Linker -> Executable Object file
```

For example: when you compile c program in linux system(assume we have two c files right now, where `main.c` call the function that from `sum.c`)

```bash
linux > gcc -Og -o prog main.c sum.c
```

1. The driver first runs the **C preprocessor**, which translates the C source file `main.c `into an ***ASCII intermediate file*** `main.i(/tmp/main.i)`

2. The dirver runs the **C compiler**(`cc1`), whch translate `main.i ` into an ***ASCII assembly-language file*** `main.s`:

   ```bash
   cc1 /tmp/main.i -Og [other arguments] -o /tmp/main.s
   ```

3. The driver runs the assembler, which translates `main.s` into a ***binary relocatable object file*** `main.o`<br>(`tmp/main.s` -> `tmp/main.o`).

3. The driver goes through the same process to generate `sum.o` 

4. Finally, it runs the ***linker program Id***, which **combines maino and sum.o**, along with the necessary system object files, to create ***the binary executable object file*** `prog`:

   ```bash
   ld -o prog [system object files and args] /tmp/main.o /tmp/sum.o
   ```

5. Run the executable file `prog`

   ```bash
   linux > ./prog
   ```

![the_driver_translate_process](./pic/the_driver_translates_process.png)

<p align="center">This figure comes from <a href = "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/13-linking.pdf">cmu-213 slide</a></p>

## 1.1 Static Linking

