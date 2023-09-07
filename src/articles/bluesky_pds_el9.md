# Bluesky PDS on Enterprise Linux 9

This is more or less the instructions from https://github.com/bluesky-social/pds tweaked slightly for RHEL 9 / RockyLinux 9 / AlmaLinux 9 / Oracle Linux 9.  As there is no x86_64 docker repo for EL9 we will use the CentOS Stream 9 repo.

    sudo dnf -y install dnf-plugins-core
    sudo dnf config-manager --add-repo https://download.docker.com/linux/centos/docker-ce.repo

    sudo dnf install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin

    sudo systemctl enable docker
    sudo systemctl start docker
    sudo docker run hello-world

    sudo mkdir /pds
    sudo mkdir --parents /pds/caddy/data
    sudo mkdir --parents /pds/caddy/etc/caddy

    dnf install vim-common     # necessary for xxd
    
# Create the config file for the caddy reverse-proxy

    cat <<CADDYFILE | sudo tee /pds/caddy/etc/caddy/Caddyfile
    {
      email you@example.com
      on_demand_tls {
		ask http://localhost:3000
	}
    }

    *.example.com, example.com {
      tls {
        on_demand
      }
      reverse_proxy http://localhost:3000
    }
    CADDYFILE

## Alternate nginx configuration

I like to use nginx instead of caddy, see further down for preventing caddy from starting.

    sudo docker ps

    upstream pds {
      server 127.0.0.1:3000;
    }
    
    location / {
            proxy_pass http://pds;
            proxy_http_version 1.1;
            proxy_set_header Upgrade $http_upgrade;
            proxy_set_header Connection "upgrade";
            proxy_set_header Host $host;
            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
            proxy_set_header X-Forwarded-Proto $scheme;
    }
    
    listen example.com:443

etc

It's a good idea to use LetsEncrypt certbot at this point to install an SSL cert:

    sudo dnf install certbot python3-certbot-nginx
    sudo certbot --nginx -d example.com

# Create the PDS env configuration file

    PDS_HOSTNAME="example.com"
    PDS_JWT_SECRET="$(openssl rand --hex 16)"
    PDS_ADMIN_PASSWORD="$(openssl rand --hex 16)"
    PDS_REPO_SIGNING_KEY_K256_PRIVATE_KEY_HEX="$(openssl ecparam --name secp256k1 --genkey --noout --outform DER | tail --bytes=+8 | head --bytes=32 | xxd --plain --cols 32)"
    PDS_PLC_ROTATION_KEY_K256_PRIVATE_KEY_HEX="$(openssl ecparam --name secp256k1 --genkey --noout --outform DER | tail --bytes=+8 | head --bytes=32 | xxd --plain --cols 32)"
    
    cat <<PDS_CONFIG | sudo tee /pds/pds.env
    PDS_HOSTNAME=${PDS_HOSTNAME}
    PDS_JWT_SECRET=${PDS_JWT_SECRET}
    PDS_ADMIN_PASSWORD=${PDS_ADMIN_PASSWORD}
    PDS_REPO_SIGNING_KEY_K256_PRIVATE_KEY_HEX=${PDS_REPO_SIGNING_KEY_K256_PRIVATE_KEY_HEX}
    PDS_PLC_ROTATION_KEY_K256_PRIVATE_KEY_HEX=${PDS_PLC_ROTATION_KEY_K256_PRIVATE_KEY_HEX}
    PDS_DB_SQLITE_LOCATION=/pds/pds.sqlite
    PDS_BLOBSTORE_DISK_LOCATION=/pds/blocks
    PDS_DID_PLC_URL=https://plc.bsky-sandbox.dev
    PDS_BSKY_APP_VIEW_URL=https://api.bsky-sandbox.dev
    PDS_BSKY_APP_VIEW_DID=did:web:api.bsky-sandbox.dev
    PDS_CRAWLERS=https://bgs.bsky-sandbox.dev
    PDS_CONFIG

# Start the PDS containers

### Download the compose.yaml to run your PDS, which includes the following containers:

* pds Node PDS server running on http://localhost:3000
* caddy HTTP reverse proxy handling TLS and proxying requests to the PDS server
* watchtower Daemon responsible for auto-updating containers to keep the server secure and federating

    curl https://raw.githubusercontent.com/bluesky-social/pds/main/compose.yaml | sudo tee /pds/compose.yaml

### Create the systemd service

    cat <<SYSTEMD_UNIT_FILE >/etc/systemd/system/pds.service
    [Unit]
    Description=Bluesky PDS Service
    Documentation=https://github.com/bluesky-social/pds
    Requires=docker.service
    After=docker.service
    
    [Service]
    Type=oneshot
    RemainAfterExit=yes
    WorkingDirectory=/pds
    ExecStart=/usr/bin/docker compose --file /pds/compose.yaml up --detach
    ExecStop=/usr/bin/docker compose --file /pds/compose.yaml down
    
    [Install]
    WantedBy=default.target
    SYSTEMD_UNIT_FILE

### Start the service

    sudo systemctl daemon-reload
    sudo systemctl enable pds
    sudo systemctl start pds
    sudo systemctl status pds
    sudo docker ps

If you are using nginx, take note of the caddy container id, and stop it from restarting:

    docker update --restart=no ded5f0df4be8b # your container ID
    docker stop ed5f0df4be8b

## Verify PDS is online

    curl https://example.com/xrpc/_health
    {"version":"0.2.2-beta.2"}


## Obtain PDS password

    $ source /pds/pds.env
    $ echo $PDS_ADMIN_PASSWORD
    a7b5970b6a5077bb41fc68a26d30adda


## Generate an invite code for your PDS

    PDS_HOSTNAME="example.com"
    PDS_ADMIN_PASSWORD="<YOUR PDS ADMIN PASSWORD>"
    
    curl --silent \
      --show-error \
      --request POST \
      --user "admin:${PDS_ADMIN_PASSWORD}" \
      --header "Content-Type: application/json" \
      --data '{"useCount": 1}' \
      https://${PDS_HOSTNAME}/xrpc/com.atproto.server.createInviteCode


## Manually updating

    sudo docker pull ghcr.io/bluesky-social/pds:latest
    sudo systemctl restart pds

