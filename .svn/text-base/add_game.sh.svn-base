#!/usr/bin/bash



TABLENAME='product_last_sync_table'
HOSTNAME='localhost'
USERNAME='root'
PASSWORD='ta0mee'
DBNAME='sync_db'
PORT='3306'


PLATFORMID=$1
GAMENAME=$2
ZONEID=$3


if [ $# -ne 3 ]
then
    printf "usage platform_id & game_id & zone_id\n"
    exit 1
fi

# generate server_key

server_key="plid:"$PLATFORMID"|gid:"$GAMENAME"|zid:"$ZONEID

echo $server_key

insert_sql="insert into ${TABLENAME} (server_key, reload_flag)values('$server_key',1)"
mysql -h${HOSTNAME} -P${PORT} -u${USERNAME} -p${PASSWORD} ${DBNAME} -e "${insert_sql}"
