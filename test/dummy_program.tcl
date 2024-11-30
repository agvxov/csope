proc f {n} {
    for {set i 0} {$i < $n} {incr i} {
        puts [format "-- (%d)" $i]
    }
}

f 10
