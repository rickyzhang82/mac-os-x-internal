-- osversion.scpt
tell application "Finder"
    set system_version to (get the version)
    say "[[emph +]]Cool. This is Mac OS Ten" & system_version
end tell
