# ==============================================================================
# PowerShell скрипт для тестирования TCP-сервера join_server
# ==============================================================================
# Инструкция:
# 1. Запусти join_server.exe <port> в отдельной консоли (например, .\join_server.exe 8000).
# 2. Сохрани этот код в файл test_server.ps1.
# 3. Открой PowerShell, перейди в папку с файлом и выполни: .\test_server.ps1
# ==============================================================================

# --- Настройки подключения ---
$serverAddress = "localhost"
$port = 8000 # Укажи тот же порт, на котором запущен твой сервер join_server.exe

# --- Создание TCP-клиента ---
# Используем ${port} для корректной интерполяции переменной в строке
Write-Host "Connecting to \(serverAddress:\){port}..."
try {
    # Пытаемся создать объект TcpClient и подключиться к серверу
    $client = New-Object System.Net.Sockets.TcpClient($serverAddress, $port)
    $stream = $client.GetStream()
    
    # --- УКАЗАНИЕ КОДИРОВКИ UTF-8 БЕЗ BOM ---
    # Используем UTF8Encoding БЕЗ Byte Order Mark (BOM) для корректной передачи данных
    $utf8Encoding = New-Object System.Text.UTF8Encoding($false) 
    $writer = New-Object System.IO.StreamWriter($stream, $utf8Encoding)
    $reader = New-Object System.IO.StreamReader($stream, $utf8Encoding)
    $writer.AutoFlush = $true # Автоматически отправлять данные после записи
    
    Write-Host "Connected successfully."
}
catch {
    # Если подключение не удалось, выводим ошибку и завершаем скрипт
    Write-Error "Failed to connect to server: $($_.Exception.Message)"
    Write-Host "Please ensure the server 'join_server.exe' is running on port $port."
    exit 1 # Выходим из скрипта
}

# --- Функция для отправки команды и получения ПОЛНОГО ответа ---
# Эта функция читает строки ответа до тех пор, пока не встретит "OK" или "ERR".
# Это важно для команд, возвращающих несколько строк (INTERSECTION, SYMMETRIC_DIFFERENCE),
# а также для команд, возвращающих ошибки.
function Send-Cmd($cmd) {
    Write-Host "Sending: '$cmd.'"
    # Отправляем команду, добавляя точку '.' как терминатор команды
    $writer.WriteLine("$cmd.")
    
    # Используем StringBuilder для эффективного сбора строк ответа
    $responseBuilder = New-Object System.Text.StringBuilder 
    $line = ""
    
    # Цикл для чтения всех строк ответа, пока не получим "OK" или "ERR"
    while ($true) {
        try {
            # Читаем одну строку
            $line = $reader.ReadLine()
            
            # Проверяем, не был ли ответ NULL (что может означать разрыв соединения)
            if ($null -eq $line) {
                Write-Error "Connection lost while reading response. Server might have closed the connection."
                break # Выходим из цикла, если соединение потеряно
            }
            
            # Добавляем прочитанную строку и символ новой строки к нашему ответу
            $responseBuilder.Append($line).Append("`n") 
            
            # Проверяем, является ли прочитанная строка завершающей "OK" или сообщением об ошибке "ERR"
            if ($line -eq "OK" -or $line.StartsWith("ERR")) {
                # Если получили OK или ERR, значит, ответ полностью прочитан, выходим из цикла
                break
            }
        }
        catch {
            # Обрабатываем возможные ошибки при чтении (например, если поток закрыт)
            Write-Error "Error reading response: $($_.Exception.Message)"
            break # Выходим из цикла при ошибке
        }
    }
    
    Write-Host "Server response:"
    # Выводим весь собранный ответ, убрав возможный лишний перевод строки в конце
    $finalResponse = $responseBuilder.ToString().TrimEnd("`n") 
    Write-Host $finalResponse
}

# --- Тестовый сценарий ---
Write-Host "`n--- Starting test scenario ---`n"

# 1. Вставляем данные в таблицы A и B согласно условию
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

# 2. Проверяем обработку дубликата ID
Write-Host "`n--- Testing duplicate key ---`n"
Send-Cmd "INSERT A 1 another_sweater" # Попытка вставить с существующим ID 1

# 3. Выполняем операции INTERSECTION и SYMMETRIC_DIFFERENCE
Write-Host "`n--- Testing operations ---`n"
Send-Cmd "INTERSECTION"
Send-Cmd "SYMMETRIC_DIFFERENCE"

# 4. Очищаем таблицу A
Write-Host "`n--- Testing TRUNCATE ---`n"
Send-Cmd "TRUNCATE A"

# 5. Проверяем, что TRUNCATE сработал, и операции после очистки корректны
Write-Host "`n--- Testing after TRUNCATE ---`n"
Send-Cmd "INSERT A 10 new_element" # Добавим элемент в A после очистки
Send-Cmd "INTERSECTION" # Ожидаем пустой результат, т.к. A была очищена и теперь содержит только 10, а B другие элементы

# --- Завершение ---
Write-Host "`n--- Test scenario finished ---`n"

# Закрываем соединение
$client.Close()
Write-Host "Connection closed."