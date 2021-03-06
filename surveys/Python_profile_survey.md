# Python Profiling Tools

+ **Deterministic profilers** can gather high-resolution profiling information but they often they have high overhead. If the application has large number of function calls,
  especially with very small execution time, the profiler will end up collecting lot of records which will result into considerable slowdown in application execution.
  In contrast, **statistical profilers** sample the effective instruction pointer to determine where the execution time is spent. These profilers also allows to adjust the
  sampling interval dynamically allowing to profile applications with very low overhead. 
  
+ cProfile: a built-in profiling module available. It is implemented as a C module based on lsprof. It uses deterministic profiling technique and can help to gather
  various metrics including total execution time, number of function calls, execution time per function, etc.
  
+ gprof2dot: a handy Python tool for converting profiler data into a dot graph. It can read profiling data generated by many other tools like cProfile, Perf, Callgrind,
  Oprofile, Intel Vtune and convert them into pretty callgraphs.
  
+ line_profiler: a Python module to perform line-by-line profiling of functions.

+ memory_profiler: a memory usage monitoring tool for Python applications and helps to understand line-by-line memory consumption of the given application.

+ Other profiler tools: http://pramodkumbhar.com/2019/05/summary-of-python-profiling-tools-part-i/
