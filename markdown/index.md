---
title: 'TiltPod No Touch'
author: '**Brian Barcenas and Emily Hoang** (website template by Ryan Tsang)'
date: '*EEC172 SQ24*'

subtitle: '<blockquote><b>EEC172 Final Project Webpage Example</b><br/>
Note to current students: this is an <i>example</i> webpage and
may not fulfill all stated requirements of the current quarter''s 
assignment.<br/>The website source is hosted 
<a href="https://github.com/ucd-eec172/project-website-example">on github</a>.
</blockquote>'

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
<a href="https://github.com/ucd-eec172/project-website-example">
  here (placeholder)</a>.

<div style="display:flex;flex-wrap:wrap;justify-content:space-evenly;padding-top:20px">
  <div style="display: inline-block; vertical-align: bottom;">
    <img src="./media/Image_001.jpg" style="width:auto;height:2in"/>
    <!-- <span class="caption"> </span> -->
  </div>
  <div style="display: inline-block; vertical-align: bottom;">
    <img src="./media/Image_002.jpg" style="width:auto;height:2in" />
    <!-- <span class="caption"> </span> -->
  </div>
</div>

<h2>Video Demo</h2>
<div style="text-align:center;margin:auto;max-width:560px">
  <div style="padding-bottom:56.25%;position:relative;height:0;">
    <iframe style="left:0;top:0;width:100%;height:100%;position:absolute;" width="560" height="315" src="https://www.youtube.com/embed/wSRtnAEZhmc?si=3vQXNj4h0WkW-F-q" title="YouTube video player" frameborder="0" allow="accelerometer; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" referrerpolicy="strict-origin-when-cross-origin" allowfullscreen></iframe>
  </div>
</div>
'
---

<!-- EDIT METADATA ABOVE FOR CONTENTS TO APPEAR ABOVE THE TABLE OF CONTENTS -->
<!-- ALL CONTENT THAT FOLLWOWS WILL APPEAR IN AND AFTER THE TABLE OF CONTENTS -->

# Market Survey

There are two types of similar product on the market. The first one is
products from AeroGarden. Their products allow users to grow plants in
nutrient solutions in a limited amount of usually 5 to 10. Compared with
this product, our product provides an automated system for nutrient
control that ensures the plant always has the correct amount of
nutrients needed to avoid excess or insufficient nutrients. The other
product is an expensive commercial system for horticulture aiming for a
large scale of growth. Compared with this one, our product has the
advantage of being cheap and small-scale which is more suitable for
individual hobbyists to explore hydroponics.

<div style="display:flex;flex-wrap:wrap;justify-content:space-evenly;">
  <div style='display: inline-block; vertical-align: top;'>
    <img src="./media/Image_003.jpg" style="width:auto;height:200"/>
    <span class="caption">
      <a href="https://aerogarden.com/gardens/harvest-family/Harvest-2.0.html">AeroGarden Harvest 2.0</a>
      <ul style="text-align:left;">
      <li>Inexpensive ($90)</li>
      <li>Not Automated</li>
      <li>Small Scale</li>
      <li>No remote monitoring</li>
    </ul>
    </span>
  </div>
  <div style='display: inline-block; vertical-align: top;'>
    <img src="./media/Image_004.jpg" style="width:auto;height:200" />
    <span class="caption">
      <a href="https://www.hydroexperts.com.au/Autogrow-MultiGrow-Controller-All-In-One-Controller-8-Growing-Zones">Autogrow Multigrow</a>
      <ul style="text-align:left;">
      <li>Expensive ($4500)</li>
      <li>Fully Automated</li>
      <li>Huge Scale</li>
      <li>Cloud monitoring</li>
    </ul>
    </span>
  </div>
</div>

# Design

## System Architecture

<div style="display:flex;flex-wrap:wrap;justify-content:space-evenly;">
  <div style="display:inline-block;vertical-align:top;flex:1 0 400px;">
    As shown in the system flowchart, we use one of the CC3200 boards for
    the closed feedback loop for maintaining concentration in the
    environment. The board will read values with I2C protocol through an ADC
    which reads values from the thermistor and the TDS meter sensor. The
    board will also periodically read user-defined thresholds from the AWS
    cloud using RESTful APIs. When the sensory values read outside of the
    user-defined thresholds, the board will activate the motor control
    function to pump either water or nutrient solution to bring the
    concentration back within the thresholds. Meanwhile, another CC3200
    board is frequently reading from the AWS cloud to present the current
    TDS reading and user-defined threshold to the user on an OLED through
    SPI protocols. To adjust the thresholds, the user can either do it
    remotely or locally by using a TV remote to type the number into the IR
    receiver. The adjustments will be updated to the AWS and if the user
    updates remotely, the local CC3200 board will update the values in the
    next synchronization.
  </div>
  <div style="display:inline-block;vertical-align:top;flex:0 0 400px;">
    <div class="fig">
      <img src="./media/Image_005.jpg" style="width:90%;height:auto;" />
      <span class="caption">System Flowchart</span>
    </div>
  </div>
</div>

## Functional Specification

<div style="display:flex;flex-wrap:wrap;justify-content:space-evenly;">
  <div style="display:inline-block;vertical-align:top;flex:1 0 300px;">
    Our system works based on the following state diagram. The device will
    periodically monitor the temperature and the electrical conductivity
    (EC) of the solution and convert the values into a TDS value using a
    calibration curve. At the same time, the device will check for threshold
    inputs, both over the AWS shadow and via manual input on the IR
    receiver. It will compare the TDS with the lower and upper thresholds
    set by the user. If the value is within the thresholds, it will stay in
    the rest state. If the TDS is higher than the upper thresholds, it will
    go to the water state and activate the water pump until the PPM is lower
    than the upper thresholds and go back to the rest state. If the PPM is
    lower than the lower thresholds, it will go to the nutrient state and
    activate the nutrient pump until the PPM is higher than the lower
    thresholds and go back to the rest state. In each state, the device will
    periodically post the TDS.
  </div>
  <div style="display:inline-block;vertical-align:top;flex:0 0 500px">
    <div class="fig">
      <img src="./media/Image_006.jpg" style="width:90%;height:auto;" />
      <span class="caption">State Diagram</span>
    </div>
  </div>
</div>

# Implementation

### CC3200-LAUNCHXL Evaluation Board

All control and logic was handled by two CC3200 microcontroller units,
one each for the Master and Slave device. On the master device, it was
responsible for decoding IR inputs from the remote to allow the user to
input thresholds to be sent over AWS. The board’s SPI functionality,
using the TI SPI library, was used to interface with the OLED display.
The MCU is WiFi enabled, allowing a remote connection between the two
boards.

On the slave device, the microcontroller was responsible for the same
functionalities as above, in addition to the TDS reading and control.
This includes interfacing with the ADC over the I2C bus, reading 
thresholds over HTTP from the AWS device shadow, writing the reported 
TDS to the device shadow, and activating the two pumps using the 
BJT control circuit.

## Functional Blocks: Master

### AWS IoT Core

<div style="display:flex;flex-wrap:wrap;justify-content:space-between;">
  <div style='display: inline-block; vertical-align: top;flex:1 0 200px'>
    The AWS IoT core allows our devices to communicate with each other
    asynchronously. The master device can update the desired thresholds, and
    the slave device will read them and synchronize them to the reported
    state. The slave device will also post the TDS and temperature readings
    periodically.
  </div>
  <div style='display: inline-block; vertical-align: top;flex:0 0 400px'>
    <div class="fig">
      <img src="./media/Image_007.jpg" style="width:auto;height:2.5in" />
      <span class="caption">Device Shadow JSON</span>
    </div>
  </div>
</div>

### OLED Display

<div style="display:flex;flex-wrap:wrap;justify-content:space-between;">
  <div style='display: inline-block; vertical-align: top;flex:1 0 400px'>
    On both Master and Slave devices, the user can view the current TDS and
    temperature of the plant solution on an OLED display. The user can also
    use the display to view and edit the TDS thresholds. The CC3200 uses the
    SPI bus to communicate with the display module.
  </div>
  <div style='display: inline-block; vertical-align: top;flex:0 0 400px'>
    <div class="fig">
      <img src="./media/Image_008.jpg" style="width:auto;height:2in" />
      <span class="caption">OLED Wiring Diagram</span>
    </div>
  </div>
</div>

### IR Receiver

<div style="display:flex;flex-wrap:wrap;justify-content:space-between;">
  <div style='display: inline-block; vertical-align: top;flex:1 0 400px'>
    On both the Master and Slave devices, a user can input the TDS
    thresholds using a TV remote. These TV remotes use the NEC code format
    with a carrier frequency of 38KHz. The Vishay IR receiver is connected
    to Pin 62 of the CC3200, which is configured as a GPIO input pin. Each
    positive edge of the signal triggers an interrupt in the main program,
    storing the pulse distances into a buffer, and allowing us to decode the
    inputs (1-9, delete and enter). The IR receiver is connected to VCC
    through a resistor and a capacitor to filter any ripples.
  </div>
  <div style='display: inline-block; vertical-align: top;flex:0 0 400px'>
    <div class="fig">
      <img src="./media/Image_009.jpg" style="width:auto;height:2in" />
      <span class="caption">IR Receiver Wiring Diagram</span>
    </div>
  </div>
</div>

## Functional Blocks: Slave

The slave device contains all the functional blocks from the master
device, plus the following:

### Analog-To-Digital Converter (ADC) Board

<div style="display:flex;flex-wrap:wrap;justify-content:space-between;">
  <div style='display: inline-block; vertical-align: top;flex:1 0 400px'>
    The outputs from the thermistor and TDS sensor board are
    in the form of analog voltages, which need to be converted to digital
    values to be usable in our program. We chose the AD1015 breakout board
    from Adafruit, which sports 4-channels and 12 bits of precision. We
    ended up using only 2 channels, so there is a potential for even more
    cost savings. The ADC board supports I2C communication, which we can use
    to request and read the two channel voltages. 
    The <a href="https://cdn-shop.adafruit.com/datasheets/ads1015.pdf">
    product datasheet</a> contains the necessary configuration values
    and register addresses for operation.
  </div>
  <div style='display: inline-block; vertical-align: top;flex:1 0 400px'>
    <div class="fig">
      <img src="./media/Image_010.jpg" style="width:auto;height:2in" />
      <span class="caption">ADC Wiring Diagram</span>
    </div>
  </div>
</div>

### Thermistor

<div style="display:flex;flex-wrap:wrap;justify-content:space-between;">
  <div style='display: inline-block; vertical-align: top;flex:1 0 300px;'>
    Conductivity-based TDS measurements are sensitive to temperature. To
    allow accurate TDS measurements in a variety of climates and seasons,
    temperature compensation calculations must be performed. To measure the
    temperature, we use an NTC thermistor connected in a voltage divider
    with a 10k resistor. The voltage across the resistor is read by the ADC
    and converted to temperature using the equation provided by the
    thermistor datasheet.
  </div>
  <div style='display: inline-block; vertical-align: top;flex:1 0 400px'>
    <div class="fig">
      <img src="./media/Image_011.jpg" style="width:auto;height:2in" />
      <span class="caption">Thermistor Circuit Diagram</span>
    </div>
  </div>
</div>

### TDS Sensor Board

<div style="display:flex;flex-wrap:wrap;justify-content:space-between;">
  <div style='display: inline-block; vertical-align: top;flex:1 0 500px'>
    In our first attempt to measure TDS, we used a simple two-probe analog
    setup with a voltage divider. We soon found out that this was a naïve
    approach (see Challenges). Consequently, we acquired a specialty TDS
    sensing board from CQRobot, which generates a sinusoidal pulse and
    measures the voltage drop to give a highly precise voltage to the ADC.
    The MCU can then convert this voltage to a TDS value using the equation
    provided by the device datasheet. We calibrated the TDS readings using a
    standalone TDS sensor pen. After calibration and setting up the curves
    for temperature compensation, we were able to achieve TDS readings
    accurate to within 5% of the TDS sensor pen.
  </div>
  <div style='display: inline-block; vertical-align: top;flex:1 0 600px'>
    <div class="fig">
      <img src="./media/Image_012.jpg" style="width:auto;height:2in;padding-top:30px" />
      <span class="caption">TDS Sensor Wiring Diagram</span>
    </div>
  </div>
</div>

### Pumps and Control Circuit

<div style="display:flex;flex-wrap:wrap;justify-content:space-between;">
  <div style='display: inline-block; vertical-align: top;flex:1 0 500px'>
    The CC3200 is unable to provide sufficient power to drive the pumps,
    which need 100mA of current each. Therefore, we used an external power
    source in the form of 2 AA batteries for each pump motor. To allow the
    CC3200 to turn on/off the motors, we designed a simple amplifier using a
    Common Emitter topology. When the control pin is asserted HIGH, the BJT
    will allow current to flow from 3V to ground through the pump motor.
    Conversely, if the control signal is LOW, the BJT will not allow current
    to flow in the motor. For each motor, there is a reverse-biased diode
    connected across it. This protects our circuit from current generated by
    the motor if it is spun from external force or inertia.
  </div>
  <div style='display: inline-block; vertical-align: top;flex:1 0 600px'>
    <div class="fig">
      <img src="./media/Image_013.jpg" style="width:auto;height:2in;padding-top:30px" />
      <span class="caption">Pump Circuit Diagrams</span>
    </div>
  </div>
</div>

# Challenges

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

Given more time, we had the idea of developing a web app to allow users
to control the device from their cell phone. Another idea we wanted to
implement in the future is adding a grow light and pH controller to
maintain a more suitable and stable environment for different plants to
grow.


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
    <td colspan="1">
      <p>TOTAL</p></td>
    <td><p>$160.99</p></td>
    <td></td>
  </tr>
</tbody>
</table>
