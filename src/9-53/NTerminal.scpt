-- NTerminal.scpt
   
tell application "Terminal"
   
    launch
   
    -- Configurable parameters
    set desiredWindowsTotal to 4
    set desiredWindowsPerRow to 2
   
    -- Ensure we have N Terminal windows: open new ones if there aren't enough
    set i to (count windows)
    repeat
        if i >= desiredWindowsTotal
            exit repeat
        end if
        do script with command "echo Terminal " & i
        set i to i + 1
    end repeat
   
    -- Adjust window positions
    set i to 1
    set j to 0
    set { x0, y0 } to { 0, 0 }
    set listOfWindows to windows
   
    repeat
        if i > desiredWindowsTotal then
            exit repeat
        end if
        tell item i of listOfWindows
            set { x1, y1, x2, y2 } to bounds
            set newBounds to { x0, y0, x0 + x2 - x1, y0 + y2 - y1 }
            set bounds to newBounds
            set j to j + 1
            set { x1, y0, x0, y1 } to bounds
            if j = desiredWindowsPerRow then -- Move to the next row
                set x0 to 0
                set y0 to y1
                set j to 0
            end if
        end tell
        set i to i + 1
    end repeat
end tell
