<h1>All-In-One-Clock-2.0</h1>
<img
  src="/Images/main.jpg"
  alt="Main"
  title="Main"
  style="display: inline-block; margin: 0 auto; width: 500px; highet: 500px">
<h1>Components</h1>
 <ul>
 <li><a href="https://store.arduino.cc/products/arduino-mega-2560-rev3">Arduino MEGA</a></li>
 <li><a href="https://store.arduino.cc/products/arduino-nano-rp2040-connect-with-headers">Arduino Nano RP2040 Connect</a></li>
 <li><a href="https://www.amazon.de/Kuman-Screen-Socket-Arduino-Function/dp/B075CXXL1M/ref=sr_1_2?ie=UTF8&qid=1536594279&sr=8-2&keywords=3.5+tft+display+arduino">3.5 TFT Touch Screen with SD Card Socket for Arduino</a></li>
 <li><a href="https://www.aliexpress.com/item/4000785325910.html?spm=a2g0o.productlist.0.0.61923f7cS5MI2w&algo_pvid=d52a4127-5191-45e6-bb08-5a729bccddbb&algo_exp_id=d52a4127-5191-45e6-bb08-5a729bccddbb-4&pdp_ext_f=%7B%22sku_id%22%3A%2210000007816863808%22%7D&pdp_pi=-1%3B0.69%3B-1%3B-1%40salePrice%3BUSD%3Bsearch-mainSearch">Buzzer</a></li>
 </ul>
<p>This is the second version of my old project: https://projecthub.arduino.cc/dzh121/0523b70f-0366-4fb2-8a01-0ca57f40c54d. </p>
<p>This project will have similar functions but have a touch screen and more information. </p>
<h1>Project Goals: </h1>
<ul>
  <li>Display Time (current time and date)</li>
  <li>Alarm clock </li>
  <li>Temperature right now </li>
  <li>Full temperature info (pressure, humidity, etc..) </li>
  <li>Forecast 3 days (today, tomorrow, and the day after tomorrow) </li>
  <li>Current weather Icon </li>
  <br>
  <li>Timer</li>
</ul>
<p>To be able to do all of this, we will need a connection to the internet. I chose for this project to use the Arduino RP2040 Connect. To use the RP2040, I will be using the https://create.arduino.cc/iot/\"\u003ehttps://create.arduino.cc/iot This official Arduino tool is very easy to use, and it's great for using the internet across multiple devices. </p> 
<p> we have an Arduino that is connected to the internet, but how will we get the information? Well, let's divide the information we need into two categories: </p>
<ul>
    <li>Time: To get the time, it's very simple because, while you are connected to the internet, you can get the Unix timestamp. The Unix timestamp is a way to count the time. How does it do it? It will count every second since January 1, 1970. For example, the time while writing it is "1682760531". To convert this time, I will use a library that will convert this time to a more readable time (hours, minutes, year, month, etc.). </li>
    <li>Temperature: To get the temperature, we will use something called an API. In short, an API is some kind of server that, when I "call it" I get JSON information back. For example, go to this link to use the NASA API that shows you the international space station location right now. http://api.open-notify.org/iss-now.json . For this project, I will use https://www.weatherapi.com. This API will have a lot of information for this project, but I will only use some of it because the Arduino cannot get so much information (we are talking about 20000 characters), so in the API settings, i changed it to only get some of the information. The API will also give me an image link, for example, https://cdn.weatherapi.com/weather/64x64/day/116.png. To get the image, I need to do a couple of things: get the image number, and determine if it's day or night. </li>
</ul>
<p>All of the information mentioned above will be sent through the ISP. The Arduino will get it and decode it</p>
<pre>temperature = doc.containsKey(\"tw\") ? doc[\"tw\"] : temperature;</pre>
<p>In this example, the Arduino will read the incoming data, and if the data has the keyword "tw"(it will be encoded as JSON) then the data will be saved in the temperature variable. </p>
<p>I also sent the image number and Day/Night but how will I show the image? Well, on the SD Card that is connected to the screen, there are two folders of all the images from day/night as bmp(bit map images). So I go to the SD Card and show the image by number or time. </p>
<p>In this project, I show icons in two ways. The first is by using a .h script, which is faster, but I can't have a lot of images like that because it takes a lot of MB. The second way is by using the SD card. </p>
<h1>Dashboard:</h1>
<p>The Arduino IoT Cloud allows us to use a dashboard. What is a dashboard? is a way for me to see the data from anywhere and on any device. 
In the dashboard, there are a lot of widgets; we will use some to show the data. We will use the button widget to have a button that will update the temperature (usually for debugging) and a couple of other widgets to show the data (temperature, humidity, pressure, and time). </p>
<img
  src="/Images/dashboard.png"
  alt="Dashboard"
  title="Dashboard"
  style="display: inline-block; margin: 0 auto; width: 500px; highet: 500px">
<h1>Fonts:</h1>
<p>I used a couple of extra fonts that I will include in the download files.</p>

<h1>The screen:</h1>
<p>I am using a 3.5-inch TFT screen; this is a touch screen. To use it, I am using the MCUFRIEND_kbv library. </p>
<p>We have 5 different screens; let's go through them one by one: </p>
<ul>
<li>Main Screen: On this screen, we can find the current time and date, the temperature, and an icon that shows the weather. Also, we have the buttons for all of the other screens. </li>
  <li>Forecast Screen: In this screen, we can see the forecast for the current day, tomorrow, and the day after tomorrow. For each day, there is an icon and a minimum and maximum temperature. </li>
  <li>Temperature screen: In this screen, we have a lot of information that includes: temperature, humidity, pressure, UV, wind speed, Feels like</li>
  <li>Alarm Screen: In this screen, we can set the alarm time by using two buttons</li>
  <li>Timer Screen: On this screen, we have a timer display where you can set the desired time and start the countdown.</li>
</ul>
<div class="flex-container">
  <img
  src="/Images/main.jpg"
  alt="Main"
  title="Main"
  style="display: inline-block; margin: 0 auto; width: 200px; highet: 200px">
  <img
  src="/Images/forecast.jpg"
  alt="Forecast"
  title="Forecast"
  style="display: inline-block; margin: 0 auto; width: 200px; highet: 200px">
  <img
  src="/Images/temp.jpg"
  alt="Temperature"
  title="Temperature"
  style="display: inline-block; margin: 0 auto; width: 200px; highet: 200px">
  <img
  src="/Images/alarm.jpg"
  alt="Alarm"
  title="Alarm"
  style="display: inline-block; margin: 0 auto; width: 200px; highet: 200px">
  <img
  src="/Images/timer.jpg"
  alt="Timer"
  title="Timer"
  style="display: inline-block; margin: 0 auto; width: 200px; highet: 200px">
</div>
<h1>Connections:</h1>
<p>In this project we don't connect a lot of thing but we do connect some: <p>
<ul>
  <li>Connect both Arduino's Using ISP. From the rp2040 A4-D21 and A5-D20</li>
  <li>From the mega 5v to RP2040 VIN</li>
  <li>From the mega gnd to RP2040 GND</li>
  <li>From the mega 5v to vcc buzzer</li>
  <li>From the mega gnd to gnd buzzer</li>
  <li>From the mega D44 to buzzer </li>
</ul>
<img
  src="/Images/schem.png"
  alt="Schematics"
  title="Schematics"
  style="display: inline-block; margin: 0 auto; width: 400px; highet: 400px">
  
<h1>Box:</h1>
<p>There is an extra 3d model for the box I used for the screen. </p>
<h1>Arduino Project Hub:</h1>
<p>You can find The prject in <a href="https://projecthub.arduino.cc/dzh121/all-in-one-clock-20-e16d7a">The Arduino project Hub</a> </p>

