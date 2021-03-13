puts "build app"
setws .
app config -name rmgmt -add include-path ../src/include

app build -name rmgmt 
