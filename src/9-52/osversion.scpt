-- osversion.scpt
tell application "Finder"
    -- get "raw" version
    set the version_data to system attribute "sysv"
   
    -- get the 'r' in MN.m.r, where MN=major, m=minor, r=revision
    set the revision to ((version_data mod 16) as string)
   
    -- get the 'm' in MN.m.r
    set version_data to version_data div 16
    set the minor to ((version_data mod 16) as string)
   
    -- get the 'N' in MN.m.r
    set version_data to version_data div 16
    set the major to ((version_data mod 16) as string)
    -- get the 'M' in MN.m.r
    set version_data to version_data div 16
    set major to ((version_data mod 16) as string) & major
    
    -- paste it all together
    set the os_version to major & "." & minor & "." & revision
    set the message to "This is Mac OSX " & os_version
   
    say message
    return os_version
end tell
