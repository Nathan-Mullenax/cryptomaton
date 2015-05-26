# cryptomaton
Weird little virtual machine thing to search for convoluted derivations of a specified string.

Input consists of a series of non-negative integers terminated by -1. 

These numbers are places on a queue. At every step, a value is removed from the front of the queue. The value is then split into bitfields that select both the target machine and three microinstructions.

The least significant 8 bit specify which machine is targeted for modification.

## Machines
There are 256 machines, each with one 16 bit unsigned register.

## Microinstructions 
With the remaining 8 bits of data from the value selected from the front of the queue, three instructions are selected. Instructions are as follows:
```
0: V = (1 + V%2 + V*2 + V%3 + V%17) xor V
1: V = V - 1
2: V = V * 3
3: V = V >> 1 (shifted right by 1)
4: V = bitwise inverse (V)
5: V = V xor (V*7%257)
6: V = V^2
7: V = V + 1
```
The first two instructions are 'extended' and can take values from 0 to 7 (3 bits).
The last instruction may only include instructions 0 through 3 (2 bits).

## Queue Values
For handy reference, queue values are interpreted thusly:
CCBBBAAAMMMMMMMM
Where A, B, and C are the instruction bitfields and M is the target machine address.

## One Simulation Step
A single step involved extracting one value from the queue, applying the three microinstructions that the value specifies to the appriopriate machine, and then placing the computed machine value to the back of the queue. Since computed values become instructions upon cycling through the queue, all code is inherently self modifying. 

## Observations
```
- All non-null inputs seems to produce results that don't cycle in the short term. We observe this by searching for a specified string among the memory cells and halting when it occurs. For longer strings (6 characters or more) this process can take hundreds of thousands of simulation steps to produce.