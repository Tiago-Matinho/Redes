
# Client-Broker Communication Over TCP

The task was to create a system of applications that capture,
process and make available data regarding the air quality of a given location.

The system is made by four applications:
- A [***broker***](https://en.wikipedia.org/wiki/Message_broker), receives and stores readings from the sensors connected to it, also shares with the sensors firmware updates
- A ***sensor***, sends to the broker every 10s the readings of the quality of the air
- A **public client**, used to access the broker readings
- An **admin client**, used to disconnect sensors, and send firmware updates to the broker so that every sensor connected to it can update itself.

This project made use of the [***select***](https://www.man7.org/linux/man-pages/man2/select.2.html) function and the [***Transmission Control Protocol***](https://en.wikipedia.org/wiki/Transmission_Control_Protocol).

