olat=[ 20.72336934032,20.7233023813,20.72423412606,20.72236571908,20.72462496038,20.72294237739,20.72445108791]
olon=[-103.31552067475,-103.31622699612,-103.31599833373,-103.31610317635,-103.31677520191,-103.31709954418,-103.31632661965]

lat=[20.7233,20.7233,20.7242,20.7223,20.7246,20.7229,20.7244]
lon=[-103.3155,-103.3162,-103.3159,-103.3161,-103.3167,-103.3170,-103.3163]

dlat=[ 723369,723302,724234,722365,724624,722942,724451]
dlon=[ 315520,316226,315998,316103,316775,317099,316326]
rssi=[-55,-73,-72,-70,-68,-75,-73]

def Weight(val):
    return 100+val

def average(*args):
    sum=0
    i=0
    for n in args:
        sum=sum+n[i];
        i+=1
    return sum/i;

def weighted_average(points = [], weight = []):
    sum=0
    temp=0
    t_weight=0
    weight_sum=0
    i=0
    for n in points:
        t_weight=Weight(weight[i])
        temp = (n*t_weight)
        sum=sum+temp
        weight_sum = weight_sum +t_weight
        i+=1
    print "total sum is: "+str(sum)
    print "weight_sum: "+str(weight_sum)
    return sum/weight_sum;
#print Weight(rssi[0])
average_lat=weighted_average(lat,rssi)
avergage_lon=weighted_average(lon,rssi);

daverage_lat=weighted_average(dlat,rssi)
davergage_lon=weighted_average(dlon,rssi);

print "floating point"
print str(average_lat)+","
print str(avergage_lon)
print "decimated"
print "20"+"."+str(daverage_lat)+ ","
print "-103"+"."+str(davergage_lon)



#plat = (lat1+lat2)/2
#plon = (lon1 + lon2)/2

#pplat = (lat1*w1+lat2*w2)/(w1+w2)
#pplon = (lon1*w1+lon2*w2)/(w1+w2)
#print pplat
#print pplon
