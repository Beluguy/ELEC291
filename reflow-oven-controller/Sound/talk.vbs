Dim  oVoice
Set oVoice = CreateObject("SAPI.SpVoice")
Set oVoice.Voice = oVoice.GetVoices.Item(0)
oVoice.Rate = -1
oVoice.Volume = 100
oVoice.Speak "zero, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 30, 40, 50, a-m, p-m"


