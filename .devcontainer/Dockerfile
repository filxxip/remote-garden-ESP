FROM espressif/idf:v5.3.1

RUN apt-get update && apt-get install -y \
    git \
    vim \
    bluez \
    && rm -rf /var/lib/apt/lists/*

RUN /opt/esp/idf/install.sh esp32

ENV IDF_PATH=/opt/esp/idf
ENV IDF_TOOLS_PATH=/opt/esp

RUN echo 'source /opt/esp/idf/export.sh > /dev/null 2>&1' >> /root/.bashrc

WORKDIR /workspace

CMD ["/bin/bash"]