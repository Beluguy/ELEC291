'https://stackoverflow.com/questions/20498004/how-to-save-sapi-text-to-speech-to-an-audio-file-in-vbscript

Dim WavFileName, StringToConvert
Dim oFileStream, oVoice

'NORMAL for values between 0 and 39
'stereo = add 1
'16-bit = add 2
'8KHz = 4
'11KHz = 8
'12KHz = 12
'16KHz = 16
'22KHz = 20
'24KHz = 24
'32KHz = 28
'44KHz = 32
'48KHz = 36

StringToConvert = InputBox("Enter String to Convert:", "Convert String to WAV")
WavFileName = InputBox("Enter Filename to save to:", "Convert String to WAV")

Const WavFormat= 20 ' 22KHz sampling rate, 8-bit, mono
Const SSFMCreateForWrite = 3 ' Creates file even if file exists and so destroys or overwrites the existing file

Set oFileStream = CreateObject("SAPI.SpFileStream")
oFileStream.Format.Type = WavFormat
oFileStream.Open WavFileName, SSFMCreateForWrite

Set oVoice = CreateObject("SAPI.SpVoice")
oVoice.Rate = 0
oVoice.Volume = 100
Set oVoice.AudioOutputStream = oFileStream
oVoice.Speak StringToConvert

oFileStream.Close
