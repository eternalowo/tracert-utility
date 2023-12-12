<h1>Windows 'tracert' utillity using C++, winsock2 and ICMP library</h1> 

(IPv4 only)

<h2>To do:</h2>
<ul>
  <li>Improve comand prompt args handling</li>
  <li>Refactor code so it looks more C++-like than C</li>
  <li>Check all possible errors situations</li>
</ul>

<h2>Testing:</h2>

'my_tracert' - is environment variable, a path to .exe file
<br>
![image](https://github.com/eternalowo/tracert-utility/assets/98911288/e4f56143-7217-40ab-b7b0-41e76a941951)
<br>
<h4>Without flags</h4>

![image](https://github.com/eternalowo/tracert-utility/assets/98911288/57d75de5-2ed1-45ab-8e05-f28b39802b49)
<h4>With -h flag</h4>

![image](https://github.com/eternalowo/tracert-utility/assets/98911288/e398b262-fa03-4a47-a765-856da60e893b)
<h4>With -w flag</h4>

![image](https://github.com/eternalowo/tracert-utility/assets/98911288/7c8433f0-fd52-4f6e-814e-60a2d713809c)
<h4>With -d flag</h4>

![image](https://github.com/eternalowo/tracert-utility/assets/98911288/5a72ecec-3d69-44a4-871a-219549a73e20)
<h4>With all flags</h4>

![image](https://github.com/eternalowo/tracert-utility/assets/98911288/de098cc5-7cf9-4f77-a87a-027c03f1d179)
<h4>Using hostname that does not exists</h4>

![image](https://github.com/eternalowo/tracert-utility/assets/98911288/f0342986-8957-4e6c-bd75-54e71694d194)
<h4>Uncorrect flag usage</h4>

![image](https://github.com/eternalowo/tracert-utility/assets/98911288/cd36eacc-70c0-4ce1-8ff7-9965d8870b0e)
![image](https://github.com/eternalowo/tracert-utility/assets/98911288/cdbee651-cd60-4cdb-b81d-7bed9ba09f91)

well, thats it
<br>
<br>
Resources:
<br>
https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol
<br>
https://www.firewall.cx/networking/network-protocols/icmp-protocol/icmp-echo-ping.html
<br>
https://www.firewall.cx/networking/network-protocols/icmp-protocol/icmp-time-exceeded.html
<br>
https://learn.microsoft.com/en-us/windows/win32/api/icmpapi/nf-icmpapi-icmpsendecho
<br>
https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/tracert
<br>
https://en.wikipedia.org/wiki/Traceroute
