#!/bin/bash
HMSTR=../hamster
rm -f *.dat

okayesno()
{
    echo -n "OK? (j/n)"
    read ok
    if [ "$ok" \= "n" ]; then
        echo "Fehler in Test $1" >>protocol.txt
    fi
}


check_interactive()
{
	testno=$1
	expect=$2
	cmd=$3
	errmsg=$4
    received=$(($cmd) 2>&1)
	echo "*********************************************"
	echo "** $testno cmd: '$cmd' Erwartet wird:"
	echo "$expect"
	echo "** $testno Erhalten wurde:"
	echo "$received"
	echo "*********************************************"
	okayesno "$testno $errmsg"
}

check_noninteractive()
{
	testno=$1
	expect=$2
	cmd=$3
	errmsg=$4
    received=$(($cmd) 2>&1)
	echo "$received" >__received.txt
	echo "$expect">__expected.txt
	difference=`diff -b __received.txt __expected.txt`
	rm -f __received.txt __expected.txt	
	if [ "$difference" ] ; then
		echo "*********************************************"
		echo "** $testno: cmd: '$cmd' Descrepancies:"
		echo "$difference"
		echo "*********************************************"
		okayesno "$testno $errmsg"
	fi
}


check=check_interactive

for i in "$@"
do
case $i in
    -n|--noninteractive)
	check=check_noninteractive
    shift # past argument
    ;;
    -i|--interactive)
	check=check_interactive
    shift # past argument
    ;;
    --default)
    shift # past argument with no value
    ;;
    *)
          # unknown option
    ;;
esac
done


echo "*********************************************"
echo "**               Hamstertest               **"
echo "*********************************************"
sleep 1

echo "*********************************************"
echo "**       0    RTFM-Nachricht               **"
echo "*********************************************"
testno="0"
read -r -d '' rtfm <<'EOF'
Usage: ../hamster {<Option>} <param1> {<param2>}
Function: Hamster management
Optionen:
     -l {<owner>}                    - show current list of hamsters
     -n <owner> <hamster> [<treats>] - add new hamster
     -f <owner> <hamster> <treats>   - feed treats to hamster
     -s <owner> <hamster>            - how is my hamster doing?
     -b <owner>                      - the bill please!
EOF
cmd="$HMSTR"
$check "$testno" "$rtfm" "$cmd" "Keine Args -> kein RTFM"


echo "*********************************************"
echo "**       1    Normale Benutzung            **"
echo "*********************************************"

$HMSTR -n schmidt baggins 17
$HMSTR -n mueller mueller 22
$HMSTR -n meier meier

sleep 4

testno="1.1"
read -r -d '' expect <<'EOF'
Owner		Name	Price	treats left
schmidt	baggins	17 €	17
mueller	mueller	17 €	22
meier	meier	17 €	0
EOF
cmd="$HMSTR -l"
$check "$testno" "$expect" "$cmd" "-l Fehler"


testno="1.2"
read -r -d '' expect <<'EOF'
Owner		Name	Price	treats left
mueller	mueller	17 €	22
EOF
cmd="$HMSTR -l mueller"
$check "$testno" "$expect" "$cmd" "-l Name Fehler"


testno="1.3.1"
expect="Done! 14 treats remaining in store"
cmd="$HMSTR -f schmidt baggins 3"
$check "$testno" "$expect" "$cmd" "-f Fehler"

testno="1.3.2"
read -r -d '' expect <<'EOF'
Owner		Name	Price	treats left
schmidt	baggins	17 €	14
EOF
cmd="$HMSTR -l schmidt"
$check "$testno" "$expect" "$cmd" "-f Fehler"


testno="1.4"
read -r -d '' expect <<'EOF'
mueller's hamster mueller has done > 0 hamster wheel revolutions,
and has 22 treats left in store. Current price is 18 €
EOF
cmd="$HMSTR -s mueller mueller"
$check "$testno" "$expect" "$cmd" "-s Fehler"

testno="1.5"
read -r -d '' expect <<'EOF'
meier has to pay 17 €
EOF
cmd="$HMSTR -b meier"
$check "$testno" "$expect" "$cmd" "-b Name Fehler"


testno="1.6"
read -r -d '' expect <<'EOF'
Owner		Name	Price	treats left
schmidt	baggins	17 €	14
mueller	mueller	18 €	22
EOF
cmd="$HMSTR -l"
$check "$testno" "$expect" "$cmd" "-l nach löschen -> Fehler"


echo "*********************************************"
echo "**       2    Falsche Kommandos            **"
echo "*********************************************"

sleep 1

testno="2.1"
expect="$rtfm"
cmd="$HMSTR"
$check "$testno" "$expect" "$cmd" "keine Option -> kein Fehler"


testno="2.2"
expect="$rtfm"
cmd="$HMSTR -u"
$check "$testno" "$expect" "$cmd" "ungueltige Option -> kein Fehler"


testno="2.3"
expect="$rtfm"

cmd="$HMSTR -l blah blubb"
$check "$testno" "$expect" "$cmd" "-l zuviele args -> kein Fehler"


testno="2.4"
expect="$rtfm"

cmd="$HMSTR -n blah ratz 22 fatz"
$check "$testno" "$expect" "$cmd" "-n zuviele args -> kein Fehler"


testno="2.5"
expect="$rtfm"
cmd="$HMSTR -n blah"
$check "$testno" "$expect" "$cmd" "-n zu wenige args -> kein Fehler"


testno="2.6"
expect="$rtfm"
cmd="$HMSTR -n"
$check "$testno" "$expect" "$cmd" "-n keine args -> kein Fehler"


testno="2.7"
expect="$HMSTR: Not a number: fatz"
cmd="$HMSTR -n blah ratz fatz"
$check "$testno" "$expect" "$cmd" "-n keine Zahl -> kein Fehler"


testno="2.8"
expect="$rtfm"
cmd="$HMSTR -f blah ratz 22 fatz"
$check "$testno" "$expect" "$cmd" "-f zuviele args -> kein Fehler"


testno="2.9"
expect="$rtfm"
cmd="$HMSTR -f blah ratz"
$check "$testno" "$expect" "$cmd" "-f zu wenige args -> kein Fehler"


testno="2.10"
expect="$rtfm"
cmd="$HMSTR -f blah"
$check "$testno" "$expect" "$cmd" "-f zu wenige args -> kein Fehler"


testno="2.11"
expect="$rtfm"
cmd="$HMSTR -f"
$check "$testno" "$expect" "$cmd" "-f keine args -> kein Fehler"


testno="2.12"
expect="$HMSTR: Not a number: fatz"
cmd="$HMSTR -f blah ratz fatz"
$check "$testno" "$expect" "$cmd" "-f keine Zahl -> kein Fehler"


testno="2.13"
expect="$rtfm"
cmd="$HMSTR -s blah ratz fatz"
$check "$testno" "$expect" "$cmd" "-s zuviele args -> kein Fehler"


testno="2.14"
expect="$rtfm"
cmd="$HMSTR -s blah"
$check "$testno" "$expect" "$cmd" "-s zu wenige args -> kein Fehler"


testno="2.15"
expect="$rtfm"
cmd="$HMSTR -s"
$check "$testno" "$expect" "$cmd" "-s keine args -> kein Fehler"


testno="2.16"
expect="$rtfm"
cmd="$HMSTR -b blah ratz"
$check "$testno" "$expect" "$cmd" "-b zuviele args -> kein Fehler"


testno="2.17"
expect="$rtfm"
cmd="$HMSTR -b"
$check "$testno" "$expect" "$cmd" "-b keine args -> kein Fehler"


echo "*********************************************"
echo "**      3    Datenbezogene Tests           **"
echo "*********************************************"

sleep 1

testno="3.1"
expect="No hamsters matching criteria found"
cmd="$HMSTR -l motzki"
$check "$testno" "$expect" "$cmd" "-l unbek Kunde -> kein Fehler"

testno="3.2.1"
expect="schmidt has to pay 17 €"
cmd="$HMSTR -b schmidt"
$check "$testno" "$expect" "$cmd" "-b Fehler"


testno="3.2.2"
expect="mueller has to pay 18 €"
cmd="$HMSTR -b mueller"
$check "$testno" "$expect" "$cmd" "-b Fehler"

testno="3.2.3"
expect="No hamsters matching criteria found"
cmd="$HMSTR -l"
$check "$testno" "$expect" "$cmd" "-l keine Hamster, trotzdem Anzeige"

testno="3.3.1" 
expect="Done!"
cmd="$HMSTR -n Paus Backenbube"
$check "$testno" "$expect" "$cmd" "-n Fehler"

testno="3.3.2" 
expect="In function hmstr_new(): Error: a hamster by that owner/name already exists "
cmd="$HMSTR -n Paus Backenbube"
$check "$testno" "$expect" "$cmd" "-n Hamster schon da -> kein Fehler"

testno="3.4.1"
expect="Done!"
cmd="$HMSTR -n meier meier"
$check "$testno" "$expect" "$cmd" "-n geht nicht"

testno="3.4.2"
expect="Done!"
cmd="$HMSTR -n mueller mueller"
$check "$testno" "$expect" "$cmd" "-n geht nicht"

testno="3.5.1"
expect="In function hmstr_new(): Error: a hamster by that owner/name already exists"
cmd="$HMSTR -n meier meier"
$check "$testno" "$expect" "$cmd" "-n Hamster schon da -> kein Fehler"

testno="3.5.2"
expect="In function hmstr_new(): Error: a hamster by that owner/name already exists"
cmd="$HMSTR -n mueller mueller"
$check "$testno" "$expect" "$cmd" "-n Hamster schon da -> kein Fehler"

testno="3.6" 
expect="In function hmstr_givetreats(): Error: A hamster or hamster owner could not be found"
cmd="$HMSTR -f Blott OnTheLandscape 222"
$check "$testno" "$expect" "$cmd" "-f unbek Kunde -> kein Fehler"


testno="3.7 Erwartet wird: "
expect="In function hmstr_howsdoing(): Error: A hamster or hamster owner could not be found"
cmd="$HMSTR -s Blott OnTheLandscape"
$check "$testno" "$expect" "$cmd" "-s unbek Kunde -> kein Fehler"

testno="3.8"
expect="In function hmstr_collect(): Error: A hamster or hamster owner could not be found"
cmd="$HMSTR -b Blott"
$check "$testno" "$expect" "$cmd" "-b unbek Kunde -> kein Fehler"


echo "*********************************************"
echo "**      4   zu lange Namen                 **"
echo "*********************************************"

sleep 1

testno="4.1"
expect="In function hmstr_directory(): Error: the specified name is too long"
cmd="$HMSTR -l Ludovic_Freihherr_von_Knoblauch_zu_Hatzbach"
$check "$testno" "$expect" "$cmd" "-l Langer Name -> kein Fehler"


testno="4.2"
expect="In function hmstr_new(): Error: the specified name is too long"
cmd="$HMSTR -n Ludovic_Freihherr_von_Knoblauch_zu_Hatzbach Paus_Backenbube_von_Knoblauch_zu_Hatzbach"
$check "$testno" "$expect" "$cmd" "-n zwei Lange Namen -> kein Fehler"


testno="4.3"
expect="In function hmstr_new(): Error: the specified name is too long"
cmd="$HMSTR -n Ludovic_Freihherr_von_Knoblauch_zu_Hatzbach Paus_Backenbube"
$check "$testno" "$expect" "$cmd" "-n ein Langer Name -> kein Fehler"


testno="4.4"
expect="In function hmstr_new(): Error: the specified name is too long"
cmd="$HMSTR -n Ludo Paus_Backenbube_von_Knoblauch_zu_Hatzbach"
$check "$testno" "$expect" "$cmd" "-n ein Langer Name -> kein Fehler"


testno="4.5"
expect="In function hmstr_givetreats(): Error: the specified name is too long"
cmd="$HMSTR -f Ludovic_Freihherr_von_Knoblauch_zu_Hatzbach Paus_Backenbube_von_Knoblauch_zu_Hatzbach 234"
$check "$testno" "$expect" "$cmd" "-f zwei Lange Namen -> kein Fehler"

testno="4.5"
expect="In function hmstr_givetreats(): Error: the specified name is too long"
cmd="$HMSTR -f Ludovic_Freihherr_von_Knoblauch_zu_Hatzbach Paus_Backenbube 567"
$check "$testno" "$expect" "$cmd" "-f ein Langer Name -> kein Fehler"

testno="4.6"
expect="In function hmstr_givetreats(): Error: the specified name is too long"
cmd="$HMSTR -f Ludo Paus_Backenbube_von_Knoblauch_zu_Hatzbach 789"
$check "$testno" "$expect" "$cmd" "-f ein Langer Name -> kein Fehler"

testno="4.7"
expect="In function hmstr_howsdoing(): Error: the specified name is too long"
cmd="$HMSTR -s Ludovic_Freihherr_von_Knoblauch_zu_Hatzbach Paus_Backenbube_von_Knoblauch_zu_Hatzbach"
$check "$testno" "$expect" "$cmd" "-s zwei Lange Namen -> kein Fehler"

testno="4.7"
expect="In function hmstr_howsdoing(): Error: the specified name is too long"
cmd="$HMSTR -s Ludovic_Freihherr_von_Knoblauch_zu_Hatzbach Paus_Backenbube"
$check "$testno" "$expect" "$cmd" "-s ein Langer Name -> kein Fehler"

testno="4.8"
expect="In function hmstr_howsdoing(): Error: the specified name is too long"
cmd="$HMSTR -s Ludo Paus_Backenbube_von_Knoblauch_zu_Hatzbach"
$check "$testno" "$expect" "$cmd" "-s ein Langer Name -> kein Fehler"

testno="4.9"
expect="In function hmstr_collect(): Error: the specified name is too long"
cmd="$HMSTR -b Ludovic_Freihherr_von_Knoblauch_zu_Hatzbach"
$check "$testno" "$expect" "$cmd" "-b Langer Name -> kein Fehler"
