"""
kconvert.py: Converts millivots to degrees Celcius and viceversa for K-type thermocouple.
By Jesus Calvino-Fraga 2013-2016
Constants and functions from http://srdata.nist.gov/its90/download/type_k.tab

To use in your Python program:

import kconvert
print "For 8.15 mV with cold junction at 22 C, temperature is: ", round(kconvert.mV_to_C(8.15, 22.0),2), "C"

"""
import math

# Evaluate a polynomial in reverse order using Horner's Rule,
# for example: a3*x^3+a2*x^2+a1*x+a0 = ((a3*x+a2)x+a1)x+a0
def PolyEval(lst, x):
    total = 0
    for a in reversed(lst):
        total = total*x+a
    return total
    
# -200 C to 0 C: -5.891 mV to 0 mV
mV_to_C_1 = (+0.0000000E+00, +2.5173462E+01, -1.1662878E+00, -1.0833638E+00,
             -8.9773540E-01, -3.7342377E-01, -8.6632643E-02, -1.0450598E-02,
             -5.1920577E-04, +0.0000000E+00)
#  0 C to 500 C: 0 mV to 20.644 mV
mV_to_C_2 = (+0.0000000E+00, +2.5083550E+01, +7.8601060E-02, -2.5031310E-01,
             +8.3152700E-02, -1.2280340E-02, +9.8040360E-04, -4.4130300E-05,
             +1.0577340E-06, -1.0527550E-08)
# 500 C to 1372 C: 20.644 mV to 54.886 mV
mV_to_C_3 = (-1.3180580E+02, +4.8302220E+01, -1.6460310E+00, +5.4647310E-02,
             -9.6507150E-04, +8.8021930E-06, -3.1108100E-08, +0.0000000E+00,
             +0.0000000E+00, +0.0000000E+00)
ranges_mV_to_C = (-5.891, 0.0, 20.644, 54.886)

def mV_to_C(mVolts, ColdJunctionTemp):
    total_mV=mVolts+C_to_mV(ColdJunctionTemp)
    if total_mV < ranges_mV_to_C[0]:
        return -200.1 # indicates underrange
    elif total_mV > ranges_mV_to_C[3]:
        return 1372.1 # indicates overrrange
    elif total_mV < ranges_mV_to_C[1]:
        return PolyEval(mV_to_C_1, total_mV)
    elif total_mV < ranges_mV_to_C[2]:
        return PolyEval(mV_to_C_2, total_mV)
    else:
        return PolyEval(mV_to_C_3, total_mV)

#   -270 C to 0 C
C_to_mV_1 = ( +0.000000000000E+00, +0.394501280250E-01, +0.236223735980E-04, -0.328589067840E-06,
              -0.499048287770E-08, -0.675090591730E-10, -0.574103274280E-12, -0.310888728940E-14,
              -0.104516093650E-16, -0.198892668780E-19, -0.163226974860E-22)
#   0 C to 1372 C
C_to_mV_2 = ( -0.176004136860E-01, +0.389212049750E-01, +0.185587700320E-04, -0.994575928740E-07,
              +0.318409457190E-09, -0.560728448890E-12, +0.560750590590E-15, -0.320207200030E-18,
              +0.971511471520E-22, -0.121047212750E-25)
a = (+0.118597600000E+00, -0.118343200000E-03, +0.126968600000E+03)
ranges_C_to_mV = (-270.0, 0.0, 1372.0)

def C_to_mV(tempC):
    if tempC < ranges_C_to_mV[0] or tempC > ranges_C_to_mV[2]:
        raise Exception("Temperature out of range in C_to_mV()")
    if tempC < ranges_C_to_mV[1]:
        return PolyEval(C_to_mV_1, tempC)
    else:
        return PolyEval(C_to_mV_2, tempC) + a[0] * math.exp(a[1] * (tempC - a[2]) * (tempC - a[2]))

# This section tests the correctness of the functions above by converting the range of
# temperatures from -199C to 1372C to millivolts, converting the millivolts back to
# temperature and then comparing the result with the original temperature.
if __name__ == '__main__':
    Num_Fails = 0
    Worst_Error = 0.0

    for TestTemp in range(-199, 1372):                        
        ThermocoupleVoltage = C_to_mV(TestTemp) # The thermocouple's voltage in millivolts
        ComputedTemperature=mV_to_C(ThermocoupleVoltage, 0)
        Current_Error=math.fabs(TestTemp-ComputedTemperature)
        if Current_Error > Worst_Error:
            Worst_Error = Current_Error
        if Current_Error > 0.05:  # According to the table the worst error could be 0.06
            print ("Failed at emperature: ", TestTemp, "Got instead: ",  round(ComputedTemperature,2))
            Num_Fails=Num_Fails+1
    print ("Test finished with ", Num_Fails, "failures(s). Worst error was: ", round(Worst_Error, 2))
