xfce4-terminal -e "bash -c \"./broker; exec bash\"" -T "Broker"
#sensores
xfce4-terminal -e "bash -c \"./sensor 101 CO2 Lisboa 1.0; exec bash\"" -T "Sensor 101"
xfce4-terminal --tab -e "bash -c \"./sensor 01 Chuva Lisboa 1.0; exec bash\"" -T "Sensor 01"
xfce4-terminal --tab  -e "bash -c \"./sensor 103 CO2 Évora 1.0; exec bash\"" -T "Sensor 103"
xfce4-terminal --tab -e "bash -c \"./sensor 99 CH4 Lisboa 1.0; exec bash\"" -T "Sensor 99"

#cliente public
xfce4-terminal -e "bash -c \"./public_cli; exec bash\"" -T "Cliente público"
xfce4-terminal --tab -e "bash -c \"./public_cli; exec bash\"" -T "Cliente público"


#cliente admin
xfce4-terminal --tab -e "bash -c \"./admin_cli; exec bash\"" -T "Cliente admin"
