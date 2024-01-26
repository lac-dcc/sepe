#!/bin/sh

set -e

if [ $# -lt 1 ]; then
	echo "usage: $0 <key size>"
	exit 1
fi

KEY_SIZE=$1
ID=0
QUEUE=""

echo "std::size_t NaiveHash::operator()(const std::string& key) const {"

for i in $(seq 0 8 $((KEY_SIZE - 8))); do
	echo "    std::size_t var${ID} = load_u64_le(key.c_str() + $i);"

	KEY_SIZE=$((KEY_SIZE - 8))
	QUEUE="$QUEUE var$ID"
	ID=$((ID + 1))
done

if [ "$KEY_SIZE" -gt 0 ]; then
	echo "    std::size_t var${ID} = load_u64_le(key.c_str() + $(($1 - 8)));"

	QUEUE="$QUEUE var$ID"
	ID=$((ID + 1))
fi

ID=0
while [ "$(echo "$QUEUE" | wc -w)" -gt 1 ]; do
	VAR1=$(echo "$QUEUE" | awk '{print $NF}')
	QUEUE=$(echo "$QUEUE" | awk 'NF{NF-=1};1')

	VAR2=$(echo "$QUEUE" | awk '{print $NF}')
	QUEUE=$(echo "$QUEUE" | awk 'NF{NF-=1};1')

	echo "    std::size_t xor${ID} = $VAR1 ^ $VAR2;"

	QUEUE="xor$ID $QUEUE"
	ID=$((ID + 1))
done

echo "    return ${QUEUE};"
echo "}"
