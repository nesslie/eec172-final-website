---
title: 'TiltPod No Touch'
author: '**Brian Barcenas and Emily Hoang** (website template by Ryan Tsang)'
date: '*EEC172 SQ24*'


toc-title: 'Table of Contents'
abstract-title: '<h2>Description</h2>'
abstract: 'TiltPod No Touch is a smart device that enables users to access custom apps
, similarly to a smart phone. User interface involves controlling the cursor 
through the BMA222''s accelerometer and clicking on the apps via the LaunchPad''s 
switches. SW2 enables the user to click on apps and interact with its features 
if any, while SW3 allows them to return to the home screen at any given time.
The cursor is shown on the color OLED display, and is updated in real-time as 
the user moves the cursor. Two apps we implemented were a Stocks app and 
Messaging app, where the user can search a company''s stock value to potentially
purchase it, and send a message resembling a text. These texts in both apps are 
typed using a TV IR remote, which are sent to AWS IoT Cloud through the HTTP 
POST method. The Stocks app uses a Lambda function to retrieve data about the 
company''s stocks and returns its ticker, name, price, and exchange when
requested through the HTTP GET method. We process this information and 
display the ticker, price, and exchange on the OLED screen. Then, a button
will appear prompting the user to purchase it if desired.
<br/><br/>
Our source code can be found 
<!-- replace this link -->
<a href="https://github.com/nesslie/eec172-final-website/tree/main/code">
  here </a>.
<h2>Video Demo</h2>
<div style="text-align:center;margin:auto;max-width:560px">
  <div style="padding-bottom:56.25%;position:relative;height:0;">
  <iframe width="560" height="315" src="https://www.youtube.com/embed/KxDK-oURCSk?si=CIdicxX0qZjuZDgk" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" referrerpolicy="strict-origin-when-cross-origin" allowfullscreen></iframe>
  </div>
</div>
'
---

<!-- EDIT METADATA ABOVE FOR CONTENTS TO APPEAR ABOVE THE TABLE OF CONTENTS -->
<!-- ALL CONTENT THAT FOLLWOWS WILL APPEAR IN AND AFTER THE TABLE OF CONTENTS -->

# Design

## System Architecture

<div style="display:flex;flex-wrap:wrap;justify-content:space-evenly;">
  <div style="display:inline-block;vertical-align:top;flex:1 0 400px;">
    The farthest point from the CC3200 Launchpad is the IR Remote, which simply
    sends its signals through the air. These signals are picked up and converted
    to pulses by the onboard TSOP 311 Receiver, which is connected to the
    CC3200 Launchpad. The onboard switches are also connected to the CC3200
    Launchpad. Our micocontroller will have the responsibility of decoding the
    input from the IR Receiver and switches to determine internal state. Based
    on this internal state, actions to control the OLED Display and send messages
    to AWS IoT are executed. In the case of the Stocks app, a conditional wait
    to obtain a state back from AWS IoT is executed. 
  </div>
  <div style="display:inline-block;vertical-align:top;flex:0 0 400px;">
    <div class="fig">
      <img src="./media/SystemArchectireu.png" style="width:90%;height:auto;" />
      <span class="caption">System Flowchart</span>
    </div>
  </div>
</div>

## Functional Specification

<div style="display:flex;flex-wrap:wrap;justify-content:space-evenly;">
  <div style="display:inline-block;vertical-align:top;flex:1 0 300px;">
  Our entry point for the user begins when they turn on the device, where after
  some wait for initialization the home screen will render. From there, the
  cursor will update depending on the values read by the onboard accelerate
  of the device. Our user interacts with the device by manipulating switches
  which act as touch inputs. Depending on the state of these switches, the
  application changes state, which will propagate to a home screen change. When
  text input is required, the user will interact with the remote which will
  update the onboard buffer of storage asynchronously. The application 
  then becomes a consumer of the data, where it can conditionally retrieve it
  depending on the current app screen. This will then be sent to AWS, where
  AWS IoT routes messages to either a Lambda Service or SNS. AWS will then
  update the device shadow depending on the data sent, where the launchpad can
  again consume and display it, achieving the functionality required for the
  Stocks and Messaging apps.
  </div>
  <div style="display:inline-block;vertical-align:top;flex:0 0 500px">
    <div class="fig">
      <img src="./media/functional.png" style="width:90%;height:auto;" />
      <span class="caption">State Diagram</span>
    </div>
  </div>
</div>

# Implementation
## Micocontroller Logic
Although our original architecture was slightly more complex, our eventual
implementation ended up being much simpler than initially expected. The basic
idea is that on the refresh of the display, all interfaces are polled and
application-specific behavior is determined and validated. By building on top
of the OLED commands implemented from Lab 2, we could cut down on development
time and create functions which could be called to draw the app screen
depending on the state of of device at the time. This makes switching between
screens intuitive from a developer's perspective, and we can re-use the same
state variables to conditionally send data to AWS IoT. 

Control of the state variables is determined by the current status of the
cursor and the state of both of the switches onboard the CC3200 Launchpad. If
the user's cursor is over an app icon / region of interest when either switch
is activated, this will register as a button press. Attached to these button 
presses are the changes to the state variables which will get picked up on the
next refresh of the screen. This is the idea behind the asynchronous updates
to the IR Receiver buffer since the main loop can simply read from the buffer
when the state is correct, such as when in the stock app. The message is
then conditionally formatted to the app state and sent to AWS where in the case
of the stock app, it will wait for the Lambda Function to finish execution
and display the data once the Shadow State is updated.

## AWS Implementation
On the AWS Side, AWS IoT is used to handle all communication from the
Launchpad to the rest of the internet. To implement the functionality in the
Stocks app, an AWS Lambda function is used to access the API Ninjas Stock API.
Due to the way the \verb|g_app_config| struct is set up, our Launchpad had no
clear way to connect to multiple servers at the same time, so using AWS IoT
with routing allows us to bypass this limitation. Our application uses the
way AWS IoT can conditionally trigger by only reading the variables that were
changed since the last shadow topic. This allows our messaging service and our
stock query service to trigger independently of each other, rather than every
time a change to the state is made. Essentially its a makeshift version of API
Gateway.

# Challenges

## Touchscreen Interface Was Far More Difficult Than Expected
One major challenge was that we tried to implement a new touch screen device (HiLetgo ILI9341 2.8" SPI TFT LCD Display Touch Panel) instead of the OLED display for easier and better user interface experience as it comes with a built-in tap sensor. Because it uses a different driver chip, ILI9341, while the OLED uses SSD1351, we could not directly use the given Adafruit\_SSD1351.h file. However, the main issue stemmed from the fact that these Adafruit driver chip header files are written in C++ with the intent of using them with Arduinos. So, the header files needed to be manually ported from C++ to C, so that the CC3200 LaunchPad will be able to compile and run these external files successfully. As the device uses SPI communication like the OLED, we were able to set up the connections and port the necessary functions, such as the initialization sequence, by referencing the Adafruit\_SSD1351.h file and using the same writeCommand() and writeData() functions. However, the challenge came from testing this communication as the Adafruit\_GFX files are used universally in conjunction with the Adafruit driver chip header files. But because this also comes in C++ with Arduino-specific language, these files needed to be customized for the touch screen device as well. Although the given Adafruit\_GFX files were already given in C, there are commands specific to the OLED's SSD1351 driver chip that are not directly transferable to the ILI9341 driver chip. Although we tried to translate and replace those specific commands, the graphic functions would not show on the device, making it unable to determine where specifically the problem stems from. Initializing and implementing a new device takes time and if we had more time, we may have been able to set it up and use it instead.

## AWS Struggles
One of the biggest obstacles to achieving our original vision is the way
request and network requests work with the Launchpad. Since our Launchpad
lacks to the credentials to interface with webservers the way a browser
or an API testing service like Postman do, we were extremely limited with
how to build scalable web services. Our original vision was to use AWS API
Gateway since its routing and automatic integration would be perfect
for quickly adding webapp configurations and functionality. However after
configuring a RESTFul API, deploying it and making the appropriate changes
to the Launchpad Code, our micro controller just couldn't seem to connect. It
was most likely a permissions issue, but there wasn't a clear way to attach
API Gateway access to our certificates, so we ended up pivoting to using 
AWS IoT Rules instead.

## TI SDK Documentation out of sync
Another surprising struggle was serializing JSON responses on the Launchpad.
There was a well documented example on TI's reference online, but it turns out
that the implementation on the documentation, and the implementation included 
in the SDK were completely different from each other, leading to much
confusion and trying different lightweight C JSON libraries. Eventually we
realized that the parser was not consuming the response as we expected it, but
it took a very long time to reach that point, and therefore we ran out of time
to add other apps.

## Paid API Access
And finally, API access to the Twitter has been slashed in recent years, with
no clear messages on what's a paid feature and what's supported on the free
tier. So unfortunately after a while of trying to make it work, we had to
scrap the idea.
# Future Work

Some additional features we can add include implementing a vertical or horizontal scroll to realistically mimic a smart device, adding a music player app through music streaming APIs such as Spotify, and enhancing user interface by smoothing the app screen transitions and cursor control. Another feature we could add is a two-way communication between two LaunchPads so that they send and receive messages on the Messaging app through AWS IoT.
Finally, we could simplify memory usage by switching to an MQTT protocol
for two-way communication between the board and AWS IoT.


# Finalized BOM

<!-- you can convert google sheet cells to html for free using a converter
  like https://tabletomarkdown.com/convert-spreadsheet-to-html/ -->

<table style="border-collapse:collapse;">
<thead>
  <tr>
    <th><p>Product</p></th>
    <th><p>Qty</p></th>
    <th><p>Price</p>/p></th>
    <th><p>Retailer</p></th>
    <th><p>Role</p></th>
  </tr>
</thead>
<tbody>
  <tr>
    <td><p>CC3200-LAUNCHXL</p></td>
    <td><p>1</p></td>
    <td><p>$66.00</p></td>
    <td><p>Mouser</p></td>
    <td><p>Central Control for Peripherals and Data Processing</p></td>
  </tr>
  <tr>
    <td><p>Adafruit 128x128 OLED RGB Color Display SSD1531</p></td>
    <td><p>1</p></td>
    <td><p>$39.95</p></td>
    <td><p>Adafruit</p></td>
    <td><p>Displays App Screens, Cursor, and Data Input</p></td>
  </tr>
  <tr>
    <td><p>Vishay TSOP31114 IR Receiver</p></td>
    <td><p>1</p></td>
    <td><p>$1.40</p></td>
    <td><p>Pololu</p></td>
    <td><p>Receiving and Decoding IR Signals</p></td>
  </tr>
  <tr>
    <td><p>S10-S3 Remote Control Compatible with At\&t U-Verse Receivers</p></td>
    <td><p>1</p></td>
    <td><p>$53.09</p></td>
    <td><p>Amazon</p></td>
    <td><p>Sending Character Based Communication</p></td>
  </tr>
  <tr>
    <td><p>100 ohm resistor</p></td>
    <td><p>1</p></td>
    <td><p>$0.28</p></td>
    <td><p>Digikey</p></td>
    <td><p>Used for low-pass filter in IR circuit</p></td>
  </tr>
  <tr>
    <td><p>100 uF capacitor</p></td>
    <td><p>1</p></td>
    <td><p>$0.27</p></td>
    <td><p>Digikey</p></td>
    <td><p>Used for low-pass filter in IR circuit</p></td>
  <tr>
    <td colspan="1">
      <p>TOTAL PARTS</p></td>
    <td><p>6</p></td>
    <td colspan="2">
      <p>TOTAL</p></td>
    <td><p>$160.99</p></td>
  </tr>
</tbody>
</table>
