cd vmr/
rm -r src/
sed -i '$ d' .project
printf "\t<linkedResources>\n" >> .project

printf "\t\t<link>\n" >> .project
printf  "\t\t\t<name>lscript.ld</name>\n"  >> .project
printf  "\t\t\t<type>1</type>\n" >> .project
printf  "\t\t\t<locationURI>PARENT-2-PROJECT_LOC/src/lscript.ld</locationURI>\n" >> .project
printf  "\t\t</link>\n" >> .project

cd ../
cd ../src/
OUTPUT=$(find . -name "*.h")
#printf "$OUTPUT\n"
cd ../build/vmr/

while IFS= read -r line; do
        IFS='/'
        read -ra fpath <<< "$line"
        printf  "\t\t<link>\n" >> .project
        printf  "\t\t\t<name>inc/"${fpath[1]}/${fpath[2]}"</name>\n"  >> .project
        printf  "\t\t\t<type>1</type>\n" >> .project
        printf  "\t\t\t<locationURI>PARENT-2-PROJECT_LOC/src/"${fpath[1]}/${fpath[2]}"</locationURI>\n" >> .project
        printf  "\t\t</link>\n" >> .project
done <<< "$OUTPUT"
IFS=' '

cd ../../src/
OUTPUT=$(find . -name "*.c")
cd ../build/vmr/
#printf "$OUTPUT\n"

while IFS= read -r line; do
        IFS='/'
        read -ra fpath <<<  "$line"
        printf  "\t\t<link>\n" >> .project
        printf  "\t\t\t<name>src/"${fpath[1]}/${fpath[2]}"</name>\n"  >> .project
        printf  "\t\t\t<type>1</type>\n" >> .project
        printf  "\t\t\t<locationURI>PARENT-2-PROJECT_LOC/src/"${fpath[1]}/${fpath[2]}"</locationURI>\n" >> .project
        printf  "\t\t</link>\n" >> .project
        printf "$line\n"
done <<< "$OUTPUT"

IFS=' '
printf "\t</linkedResources>\n" >> .project
printf  "</projectDescription>" >> .project

