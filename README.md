# Variable-Voltage-EDF-Scheduler
This scheduler uses the widely adopted STS algorithm implemented into a a dynamic scheduler - the earliest deadline first (EDF) algorithm - to address the problem of limited power without compromising on missed deadlines.

Note that this is merely a simulator - it simulates real time arrival of tasks. There would be changes to be made in order to use this on an actual device, however the correctness is in the pudding. Provided are a set of tasks that the program will simulate their arrival (sporadically) and schedule accordingly, while limiting clock speed based on the current task queue.

This is an implementation of the research done in this IEEE paper: 

"I. Hong, M. Potkonjak, and M. B. Srivastava, “On-line scheduling of hard real-time tasks on
variable voltage processor,” Proc. Computer-Aided Design (ICCAD), pages 653–656, November
1998."
