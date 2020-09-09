#!/bin/bash
FILENAME="test.txt"

first_word=`awk '{print $1}' $FILENAME`
second_word=`awk '{print $2}' $FILENAME`
third_word=`awk '{print $3}' $FILENAME`

if [[ $first_word == 'CREATE' ]]; then 
[[ -f $third_word"_table".txt ]] && echo "Table already exists" || { touch $third_word"_table".txt; touch $third_word"_entries".txt; }
elif [[ $first_word == 'DELETE' ]]; then
[[ -f $third_word"_table".txt ]] && { rm $third_word"_table".txt; rm $third_word"_entries".txt ;} || echo "Table does not exist"
#elif [[ $first_word == 'INSERT' ]]; then
#[[ -f $third_word"_entries".txt ]] && #appenda till filen här använd tee förmodligen || echo "Table does not exist" 
#elif [[ $first_word == 'SELECT' ]]; then
#[[ -f $third_word"_entries".txt ]] && #skicka ut valda värden från filen || echo "Table does not exist"
else
echo "Wrong request, we only accept CREATE, DELETE, INSERT or SELECT"
fi

#create_table()
#delete_table()
#insert()
#select()
