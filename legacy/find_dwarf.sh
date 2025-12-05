#!/bin/bash
# DWARF II Finder - Sucht nach DWARF II im Netzwerk

echo "🔍 Suche nach DWARF II im Netzwerk..."
echo ""

# Netzwerk-Bereich ermitteln
SUBNET=$(ip route | grep default | awk '{print $3}' | cut -d. -f1-3)

if [ -z "$SUBNET" ]; then
    echo "❌ Konnte Netzwerk nicht ermitteln"
    exit 1
fi

echo "📡 Scanne Netzwerk: ${SUBNET}.0/24"
echo ""

# Alle Hosts scannen
for ip in ${SUBNET}.{1..254}; do
    # Parallel scannen für Geschwindigkeit
    (
        # Prüfe ob Host erreichbar
        if ping -c 1 -W 1 $ip &>/dev/null; then
            # Prüfe DWARF II Ports
            if nc -z -w 1 $ip 8082 2>/dev/null && \
               nc -z -w 1 $ip 9900 2>/dev/null; then
                echo "✅ DWARF II gefunden: $ip"
                echo "   Ports: 8082 (HTTP), 8092 (Stream), 9900 (WebSocket)"
                
                # Versuche Geräte-Info abzurufen
                INFO=$(curl -s -m 2 "http://$ip:8082/getdeviceInfo" 2>/dev/null)
                if [ ! -z "$INFO" ]; then
                    echo "   Info: $INFO"
                fi
            fi
        fi
    ) &
done

# Warte auf alle Background-Jobs
wait

echo ""
echo "✅ Scan abgeschlossen"
