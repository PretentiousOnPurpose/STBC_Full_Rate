clc;
clear all;

% STBC Parameters
a = 1/sqrt(2);
c = a;
b = ((1-sqrt(7)) + 1i * (1 + sqrt(7))) / (4 * sqrt(2));
d = -1i * b;

M = 4; % QAM Alphabet
cc = 0; % Channel Coding on/off
chan = 0; % Add Fading Channel
n = 0; % Add AWGN
QAM_CONST = round((sqrt(2) * qammod((0:M-1)', M, 'gray', 'UnitAveragePower', 1)) * 23170); % QAM Constellation

%% Transmitter
numTrails = 1;
SNR_Range = 0;
numErr = zeros(length(SNR_Range), 1);
for iter_trail = 1: numTrails
    for iter_snr = 1: length(SNR_Range)
        numBits = 8;
%         data = randi([0, 1], numBits, 1);
        data = [0;0;0;1;1;0;1;1];
        if (cc == 1)
            encData = lteConvolutionalEncode(data);
            encData = lteRateMatchConvolutional(encData, 2 * numBits);
        else
            encData = data;
        end
        modData = qammod(double(encData), M, 'gray', 'InputType', 'bit', 'UnitAveragePower', 1);
        modData = round((sqrt(2) * modData) * 23170);
        txSignal = zeros(2, length(modData) / 2);
        
        if (chan == 1)
            H = 1/sqrt(2) * (randn(2, 2) + 1i * randn(2, 2));
        else
%             H = ones(2, 2);    
            H = [[21;45], [78;10]];
        end
        H_hat = H; % + 0.05 * (randn(size(H)) + 1i * size(H));

        iter_qam = 1;
        for iter_sym = 1:2:length(txSignal)
            txSignal(1, iter_sym) = a * modData((iter_qam - 1) * 4 + 1, 1) + b * modData((iter_qam - 1) * 4 + 3, 1);
            txSignal(1, iter_sym + 1) = -c * conj(modData((iter_qam - 1) * 4 + 2, 1)) - d * conj(modData((iter_qam - 1) * 4 + 4, 1));
            txSignal(2, iter_sym) = a * modData((iter_qam - 1) * 4 + 2, 1) + b * modData((iter_qam - 1) * 4 + 4, 1);
            txSignal(2, iter_sym + 1) = c * conj(modData((iter_qam - 1) * 4 + 1, 1)) + d * conj(modData((iter_qam - 1) * 4 + 3, 1));
            
            iter_qam = iter_qam + 1;
        end

        %% Receiver
        Y = H * txSignal; 
        rxSignal = Y .* (1); %
        chAvg = 78;
        chPwr = sum(sum(abs(H_hat .^ 2))) / (chAvg ^ 2);
        rxSignal = rxSignal ./ chAvg;
        H_hat = H_hat ./ chAvg;
        if (n == 1)
            rxSignal = rxSignal + 1/(sqrt(2 * (10 ^ (SNR_Range(iter_snr, 1)/10)))) * (randn(size(Y)) + 1i * randn(size(Y)));
        end
        %% Decoding
        if (cc == 1)
            enc_data_hat = zeros(2 * numBits, 1);
        else 
            enc_data_hat = zeros(numBits, 1);
        end
        iter_qam = 1;
        for iter_sym = 1:2:length(rxSignal)
            r1 = rxSignal(1, iter_sym);
            r2 = rxSignal(1, iter_sym + 1);
            r3 = rxSignal(2, iter_sym);
            r4 = rxSignal(2, iter_sym + 1);

            D = zeros(M, M);

            for iter_const1 = 1: M % S3
                for iter_const2 = 1: M % S4
                    s3 = QAM_CONST(iter_const1, 1);
                    s4 = QAM_CONST(iter_const2, 1);
                    
                    z1 = r1 - b * (H_hat(1, 1) * s3 + H_hat(1, 2) * s4);
                    z2 = r2 - d * (H_hat(1, 2) * conj(s3) - H_hat(1, 1) * conj(s4));
                    z3 = r3 - b * (H_hat(2, 1) * s3 + H_hat(2, 2) * s4);
                    z4 = r4 - d * (H_hat(2, 2) * conj(s3) - H_hat(2, 1) * conj(s4));

                    s1 = (conj(H_hat(1, 1)) * z1 + conj(H_hat(2, 1)) * z3) / a + (H_hat(1, 2) * conj(z2) + H_hat(2, 2) * conj(z4)) / conj(a);
                    s2 = (conj(H_hat(1, 2)) * z1 + conj(H_hat(2, 2)) * z3) / a - (H_hat(1, 1) * conj(z2) + H_hat(2, 1) * conj(z4)) / conj(a);

%                     s1 = s1 / sum(sum(abs(H_hat) .^ 2));
%                     s2 = s2 / sum(sum(abs(H_hat) .^ 2));

                    s1 = s1 / chPwr;
                    s2 = s2 / chPwr;
    
                    z1 = (H_hat(1, 1) * (a * s1 + b * s3) + H_hat(1, 2) * (a * s2 + b * s4));
                    z2 = (-H_hat(1, 1) * (d * conj(s4) + c * conj(s2)) + H_hat(1, 2) * (d * conj(s3) + c * conj(s1)));
                    z3 = (H_hat(2, 1) * (a * s1 + b * s3) + H_hat(2, 2) * (a * s2 + b * s4));
                    z4 = (-H_hat(2, 1) * (c * conj(s2) + d * conj(s4)) + H_hat(2, 2) * (c * conj(s1) + d * conj(s3)));
                    
                    D(iter_const1, iter_const2) = abs(r1 - z1) ^ 2;
                    D(iter_const1, iter_const2) = D(iter_const1, iter_const2) + ...
                        abs(r2 - z2) ^ 2;
                    D(iter_const1, iter_const2) = D(iter_const1, iter_const2) + ...
                        abs(r3 - z3) ^ 2;
                    D(iter_const1, iter_const2) = D(iter_const1, iter_const2) + ...
                        abs(r4 - z4) ^ 2;
                end
            end

            [s3_pt, s4_pt] = find(D == min(D(:)));
            s3 = QAM_CONST(s3_pt, 1);
            s4 = QAM_CONST(s4_pt, 1);    

            z1 = r1 - b * (H_hat(1, 1) * s3 + H_hat(1, 2) * s4);
            z2 = r2 - d * (H_hat(1, 2) * conj(s3) - H_hat(1, 1) * conj(s4));
            z3 = r3 - b * (H_hat(2, 1) * s3 + H_hat(2, 2) * s4);
            z4 = r4 - d * (H_hat(2, 2) * conj(s3) - H_hat(2, 1) * conj(s4));

            s1 = (conj(H_hat(1, 1)) * z1 + conj(H_hat(2, 1)) * z3) / a + (H_hat(1, 2) * conj(z2) + H_hat(2, 2) * conj(z4)) / conj(c);
            s2 = (conj(H_hat(1, 2)) * z1 + conj(H_hat(2, 2)) * z3) / a - (H_hat(1, 1) * conj(z2) + H_hat(2, 1) * conj(z4)) / conj(c);

            s1 = s1 / sum(sum(abs(H_hat) .^ 2));
            s2 = s2 / sum(sum(abs(H_hat) .^ 2));

            s = [s1;s2;s3;s4];
            
            if (cc == 1)
                enc_data_hat((iter_qam - 1) * 4 * log2(M) + 1:iter_qam * 4 * log2(M), 1) = ...
                    qamdemod(s, M, 'gray', 'OutputType', 'approxllr');
            else
                enc_data_hat((iter_qam - 1) * 4 * log2(M) + 1:iter_qam * 4 * log2(M), 1) = ...
                    qamdemod(s, M, 'gray', 'OutputType', 'bit');    
            end
            
            iter_qam = iter_qam + 1;
        end
        
        if (cc == 1)
            enc_data_hat = lteRateRecoverConvolutional(enc_data_hat, 3 * numBits);
            data_hat = ~lteConvolutionalDecode(enc_data_hat);
%             data_hat = cell2mat(data_hat);
        else
            data_hat = enc_data_hat;
        end
        numErr(iter_snr, 1) = numErr(iter_snr, 1) + sum(bitxor(data, double(data_hat)));
        
        if (sum(bitxor(data, double(data_hat))) == 0)
            disp('Success');
        else
            disp(['Err: ', num2str(sum(bitxor(data, double(data_hat))))]);
            disp(H);
        end
        
    end
    
end
