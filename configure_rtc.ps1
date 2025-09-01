$port = New-Object System.IO.Ports.SerialPort COM3,9600,None,8,one
$port.Open()
$port.WriteLine("2509011070200")
Start-Sleep -Seconds 2
$port.Close()
Write-Host "RTC configuration sent: 2509011070200"
