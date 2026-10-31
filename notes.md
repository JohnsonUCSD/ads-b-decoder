## Test Data & Reference

The `modes1.bin` test data was obtained from the dump1090 project and is used for testing my implementation.

The file in `reference/` is the output generated from dump1090 from `modes1.bin`.
I am only using this to verify that my code works correctly. 

Source: [dump1090]("https://github.com/antirez/dump1090/tree/master")



+----------+----------+-------------+--------+------------------------+-----------+
|  DF (5)  |  CA (3)  |  ICAO (24)  | TC (5) |         ME (51)        |  PI (24)  |
+----------+----------+-------------+--------+------------------------+-+---------+

TC starts on bit 33 / 5th byte (part of ME but all the messages have it so far)

Message starts on bit 38

Packet	    Bit positions
packet[0]	1–8             
packet[1]	9–16            
packet[2]	17–24           
packet[3]	25–32           
packet[4]	33–40           
packet[5]	41–48           
packet[6]	49–56           
packet[7]	57–64           
packet[8]	65–72           65 66 67 68 69 70 71 72
packet[9]	73–80           73 74 75 76 77 78 79 80
packet[10]	81–88
packet[11]	89–96
