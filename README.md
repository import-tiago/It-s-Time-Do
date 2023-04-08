# It's Time, Do!

__It's Time, Do!__ is a system (hardware + firmware + web application) to control over the internet the on/off or programming to automatically run other devices and electronic equipments.

The inspiration for creating this project comes from the need to schedule my ~~not smart~~ washing machine to auto-start when I'm out and about to save time and just put the clothes to dry when I get home.

<img src="https://github.com/TiagoPaulaSilva/It-s-Time-Do/blob/main/Assets/WashMachinePanel.png" alt="WashingMachine" width="100%"/>

## Push Notifications
The hardware device can trigger a web push notification to notify web clients of tasks that have started or ended (either previously scheduled tasks or even tasks that have been started locally).

| ![](https://github.com/TiagoPaulaSilva/It-s-Time-Do/blob/main/Assets/WebPushNotificationStartTask.png) | ![](https://github.com/TiagoPaulaSilva/It-s-Time-Do/blob/main/Assets/WebPushNotificationFinishedTask.png) |
|--|--|
  
 ## Task History and Instant Data
 Through Firebase's Real Time Database (RTDB), it is possible to monitor instant __It's Time, Do!__ hardware device parameters, such as local date, local time, status of the scheduled task and current firmware version; as well as status of the device being controlled (in my case, the washing machine), whether or not it is working, log of all previous tasks with start and finished date and time of each one.
 <p align="center"><img src="https://github.com/TiagoPaulaSilva/It-s-Time-Do/blob/main/Assets/RTDB_Preview.png" width=70%" height="70%"></p>
  
## Circuit Diagram
<img src="https://github.com/TiagoPaulaSilva/It-s-Time-Do/blob/main/Assets/Circuit.png" alt="CircuitDiagram" width="100%"/>

### Contributing
0. Give this project a :star:
1. Create an issue and describe your idea.
2. [Fork it](https://github.com/TiagoPaulaSilva/It-s-Time-Do/fork).
3. Create your feature branch (`git checkout -b my-new-feature`).
4. Commit your changes (`git commit -a -m "Added feature title"`).
5. Publish the branch (`git push origin my-new-feature`).
6. Create a new pull request.
7. Done! :heavy_check_mark:

### License Information
<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><br /><span xmlns:dct="http://purl.org/dc/terms/" property="dct:title">It's Time, Do!</span> by <a xmlns:cc="http://creativecommons.org/ns#" href="https://twitter.com/tiagopsilvaa" property="cc:attributionName" rel="cc:attributionURL">Tiago Silva</a> is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.
