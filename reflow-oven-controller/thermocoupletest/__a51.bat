@echo off
C:
cd "\ELEC291\Elec-291\reflow-oven-controller\thermocoupletest\"
"C:\CrossIDE\Call51\Bin\a51.exe" -l "C:\ELEC291\Elec-291\reflow-oven-controller\thermocoupletest\thermo_reading.asm"
echo Crosside_Action Set_Hex_File C:\ELEC291\Elec-291\reflow-oven-controller\thermocoupletest\thermo_reading.HEX
