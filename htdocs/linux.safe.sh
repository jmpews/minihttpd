# modify ssh port
cp /etc/ssh/sshd_config /etc/ssh/sshd_config.bak
sed -i 's/#Port 22/Port 2333/g' /etc/ssh/sshd_config
sed -i 's/PermitRootLogin yes/PermitRootLogin no/g' /etc/ssh/sshd_config
service sshd restart

# add user
useradd jmpews -p @f0reverl0ve -g 0

# modify iptables
iptables-save>./iptables.rules.bak
iptables -F
iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
iptables -A OUTPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
iptables -A INPUT -p tcp --match multiport --dports 53,80,2333,7777 -j ACCEPT
# iptables -A OUTPUT -p tcp --match multiport --ports 53,80,443,7777 -j ACCEPT
iptables -A INPUT -j DROP
# iptables -A OUTPUT -j DROP
# iptables-save ./iptables.up.rules


