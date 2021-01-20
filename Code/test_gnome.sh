gnome-terminal -e "bash -c \"./broker; exec bash\"" -T "Broker"
#sensores
gnome-terminal -e "bash -c \"./sensor 101 CO2 Lisboa 1.0; exec bash\"" -T "Sensor 101"
gnome-terminal --tab -e "bash -c \"./sensor 1 Chuva Lisboa 1.0; exec bash\"" -T "Sensor 1"
gnome-terminal --tab  -e "bash -c \"./sensor 103 CO2 Évora 1.0; exec bash\"" -T "Sensor 103"
gnome-terminal --tab -e "bash -c \"./sensor 99 CH4 Lisboa 1.0; exec bash\"" -T "Sensor 99"

#cliente public
gnome-terminal -e "bash -c \"./public_cli; exec bash\"" -T "Cliente 1"


#cliente admin
gnome-terminal --tab -e "bash -c \"./admin_cli; exec bash\"" -T "Cliente 2"
