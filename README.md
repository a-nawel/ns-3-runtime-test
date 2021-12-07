# ns-3-runtime-test
UCSB CS263 Project for Fall 2021

# Member
* **Nawel Alioua**
* **Yuqing Wang**


# Project Timeline 
- [x] **10/13** before 9am **[Project Vision Statement]
- [x] **10/29** Github Friday 5 pm week 5 (**Survey week**, **Simulator installation**)
- [x] **11/05**  Github Friday 5 pm week 6 (implementation week 1)
- [x] **11/12** Github Friday 5 pm week 7 (implementation week 2)
- [x] **11/19** Github Friday 5 pm week 8 (implementation week 3, profiling + Empirical evaluation, in-class presentation slides)
- [x] **11/22** In-class presentation day
- [x] **11/26** Github Friday 5 pm week 9 (Empirical evaluation + presentation preparation week)
- [x] **12/03** Github Friday 5 pm week 10 (Documentation and final report)
- [x] **12/06**  Final report due

# Progress Logs
* **10/29**: 
> + Install NS3  
> + Add `Python_profile_survey.md` to `surveys/`. 
> + Add `ns3_basics_survey.md` to `surveys/`. 

* **11/5**: 
> + Add `scenario1.py` to `examples/` (simulated network scenario 1). 
> + Add `scenario2.py` to `examples/` (simulated network scenario 2).
> + The above two .py files use deterministic profiling tool (cProfile) in Python to examine the runtime.

* **11/12**: 
> + Add `ReadingNotes.odt` about Python binding on NS3.
> + Try out C++ profiler Oprofile and Python memory Profiler.

* **11/19**: 
> + Add Python profiler results to `results/Python` (time & memory).

* **11/26**: 
> + Use the ping6 scenario and use the system-wide profiler perf to track execution time usage of C++ and Python.
> + Upload the scenario file to `Scenario/`.
> + Evaluate the execution time difference between two languages when the number of packets sent increases.
> + Prepare for presentation slides. The slides: CS 263 presentation.pptx.


* **12/3**: 
> + Use Massif to track heap/stack memory usage and upload results (massif.out.xxx) to `results/`.
> + Finish the final report.
