$serverAddress = "localhost"
$port = 8000

Write-Host "Connecting to \(serverAddress:\){port}..."
try {
    $client = New-Object System.Net.Sockets.TcpClient($serverAddress, $port)
    $stream = $client.GetStream()
    
    $utf8Encoding = New-Object System.Text.UTF8Encoding($false) 
    $writer = New-Object System.IO.StreamWriter($stream, $utf8Encoding)
    $reader = New-Object System.IO.StreamReader($stream, $utf8Encoding)
    $writer.AutoFlush = $true
    
    Write-Host "Connected successfully."
}
catch {
    Write-Error "Failed to connect to server: $($_.Exception.Message)"
    Write-Host "Please ensure the server 'join_server.exe' is running on port $port."
    exit 1
}

function Send-Cmd($cmd) {
    Write-Host "Sending: '$cmd.'"
    $writer.WriteLine("$cmd") 
    
    $responseBuilder = New-Object System.Text.StringBuilder 
    $line = ""
    
    while ($true) {
        try {
            $line = $reader.ReadLine()
            
            if ($null -eq $line) {
                Write-Error "Connection lost while reading response. Server might have closed the connection."
                break 
            }
            
            $responseBuilder.Append($line).Append("`n") 
            
            if ($line -eq "OK" -or $line.StartsWith("ERR")) {
                break
            }
        }
        catch {
            Write-Error "Error reading response: $($_.Exception.Message)"
            break
        }
    }
    
    Write-Host "Server response:"
    $finalResponse = $responseBuilder.ToString().TrimEnd("`n") 
    Write-Host $finalResponse
}

Write-Host "`n--- Starting test scenario ---`n"

Write-Host "`n--- Populating tables ---`n"
Send-Cmd "INSERT A 0 lean"
Send-Cmd "INSERT A 1 sweater"
Send-Cmd "INSERT A 2 frank"
Send-Cmd "INSERT A 3 violation"
Send-Cmd "INSERT A 4 quality"
Send-Cmd "INSERT A 5 precision"

Send-Cmd "INSERT B 3 proposal"
Send-Cmd "INSERT B 4 example"
Send-Cmd "INSERT B 5 lake"
Send-Cmd "INSERT B 6 flour"
Send-Cmd "INSERT B 7 wonder"
Send-Cmd "INSERT B 8 selection"

Write-Host "`n--- Testing duplicate key ---`n"
Send-Cmd "INSERT A 1 another_sweater" 

Write-Host "`n--- Testing operations ---`n"
Send-Cmd "INTERSECTION"
Send-Cmd "SYMMETRIC_DIFFERENCE"

Write-Host "`n--- Testing TRUNCATE ---`n"
Send-Cmd "TRUNCATE A"

Write-Host "`n--- Testing after TRUNCATE ---`n"
Send-Cmd "INSERT A 10 new element with spaces" 
Send-Cmd "INTERSECTION" 

Write-Host "`n--- Test scenario finished ---`n"

$client.Close()