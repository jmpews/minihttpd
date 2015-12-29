# modify ssh port
sed -i 's/#Port 22/Port 2333/g' /etc/ssh/sshd_config
sed -i 's/#PermitRootLogin yes/PermitRootLogin no/g' /etc/ssh/sshd_config
service sshd restart

# add user
useradd jmpews -p @f0reverl0ve -g 0

# modify iptables
iptables-save ./iptables.rules.bak
iptables -A INPUT -p icmp -j DROP
