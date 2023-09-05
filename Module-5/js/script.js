
        // For the 1st row of table
        var element1a = document.getElementById('row17A'); // day
        var element2a = document.getElementById('row29A'); // PAYU Charge
        var element3a = document.getElementById('row19A'); // OTC
        var element4a = document.getElementById('row20A'); // Recurring Charge

        // For the 2nd row of table
        var element1b = document.getElementById('row17B'); // day
        var element2b = document.getElementById('row29B'); // PAYU Charge
        var element3b = document.getElementById('row19B'); // OTC
        var element4b = document.getElementById('row20B'); // Recurring Charge
     
        // For the 3rd row of table
        var element1c = document.getElementById('row17C'); // day
        var element2c = document.getElementById('row29C'); // PAYU Charge
        var element3c = document.getElementById('row19C'); // OTC
        var element4c = document.getElementById('row20C'); // Recurring Charge
     
        // For the 4th row of table
        var element1d = document.getElementById('row17D'); // day
        var element2d = document.getElementById('row29D'); // PAYU Charge
        var element3d = document.getElementById('row19D'); // OTC
        var element4d = document.getElementById('row20D'); // Recurring Charge


        // Parse 1
        var value1A = parseFloat(element1a.textContent.replace(/,/g, '')); // Fraction
        var value2A = parseFloat(element2a.textContent.replace(/,/g, ''));
        var value3A = parseFloat(element3a.textContent.replace(/,/g, ''));  // OTC
        var value4A = parseFloat(element4a.textContent.replace(/,/g, '')); // Recurring Charge
        // Parse 2
        var value1B = parseFloat(element1b.textContent.replace(/,/g, '')); // Fraction
        var value2B = parseFloat(element2b.textContent.replace(/,/g, ''));
        var value3B = parseFloat(element3b.textContent.replace(/,/g, ''));
        var value4B = parseFloat(element4b.textContent.replace(/,/g, '')); // Recurring Charge
        // Parse 3
        var value1C = parseFloat(element1c.textContent.replace(/,/g, ''));  // Fraction
        var value2C = parseFloat(element2c.textContent.replace(/,/g, ''));
        var value3C = parseFloat(element3c.textContent.replace(/,/g, ''));
        var value4C = parseFloat(element4c.textContent.replace(/,/g, '')); // Recurring Charge
        // Parse 4
        var value1D = parseFloat(element1d.textContent.replace(/,/g, '')); // Fraction
        var value2D = parseFloat(element2d.textContent.replace(/,/g, ''));
        var value3D = parseFloat(element3d.textContent.replace(/,/g, ''));
        var value4D = parseFloat(element4d.textContent.replace(/,/g, '')); // Recurring Charge
     
       // ------------------ CALCULATIONS -------------------------------
     
        var totalCharge1 = value2A + value3A; // 1st
        var totalCharge2 = value2B + value3B; // 1st
        var totalCharge3 = value2C + value3C; // 1st
        var totalCharge4 = value2D + value3D; // 1st
     
     //----------------------
        const LADRM = 1000; // dummy value
        const PenalityRM = 200; // dummy value
     
        var netTotal1 = totalCharge1 - LADRM - PenalityRM;
        var netTotal2 = totalCharge2 - LADRM - PenalityRM;
        var netTotal3 = totalCharge3 - LADRM - PenalityRM;
        var netTotal4 = totalCharge4 - LADRM - PenalityRM;
     
     // -----------------------------------------
     
        var FixedCharge1 = value1A*value4A;
        var FixedCharge2 = value1B*value4B;
        var FixedCharge3 = value1C*value4C;
        var FixedCharge4 = value1D*value4D;
         
     // ------------------------------------------------------
     
        var total2Charge1 = value3A + FixedCharge1;
        var total2Charge2 = value3B + FixedCharge2;
        var total2Charge3 = value3C + FixedCharge3;
        var total2Charge4 = value3D + FixedCharge4;
     
     //-----------------------------------------------------------
     
        var net2Total1 = total2Charge1 - LADRM - PenalityRM;
        var net2Total2 = total2Charge2 - LADRM - PenalityRM;
        var net2Total3 = total2Charge3 - LADRM - PenalityRM;
        var net2Total4 = total2Charge4 - LADRM - PenalityRM;
     
     //-----------------------------------------------------------
     
        // Display the result inside the <td>
        var Out1A = document.getElementById('Out1A');
        Out1A.textContent = value1A.toFixed(6);
        var Out2A = document.getElementById('Out2A');
        Out2A.textContent = totalCharge1.toFixed(2).replace(/\B(?=(\d{3})+(?!\d))/g, ","); // second row
        var Out3A = document.getElementById('Out3A');
        Out3A.textContent = netTotal1.toFixed(2).replace(/\B(?=(\d{3})+(?!\d))/g, ",");
        var Out4A = document.getElementById('Out4A');
        Out4A.textContent = FixedCharge1.toFixed(2).replace(/\B(?=(\d{3})+(?!\d))/g, ",");
        var Out5A = document.getElementById('Out5A');
        Out5A.textContent = total2Charge1.toFixed(2).replace(/\B(?=(\d{3})+(?!\d))/g, ",");
        var Out6A = document.getElementById('Out6A');
        Out6A.textContent = net2Total1.toFixed(2).replace(/\B(?=(\d{3})+(?!\d))/g, ",");
     
        var Out1B = document.getElementById('Out1B');
        Out1B.textContent = value1B.toFixed(6);
        var Out2B = document.getElementById('Out2B');
        Out2B.textContent = totalCharge2.toFixed(2);
        var Out3B = document.getElementById('Out3B');
        Out3B.textContent = netTotal2.toFixed(2).replace(/\B(?=(\d{3})+(?!\d))/g, ",");
        var Out4B = document.getElementById('Out4B');
        Out4B.textContent = FixedCharge2.toFixed(0).replace(/\B(?=(\d{3})+(?!\d))/g, ",");
        var Out5B = document.getElementById('Out5B');
        Out5B.textContent = total2Charge2.toFixed(0);
        var Out6B = document.getElementById('Out6B');
        Out6B.textContent = net2Total2.toFixed(0).replace(/\B(?=(\d{3})+(?!\d))/g, ",");
     
        var Out1C = document.getElementById('Out1C');
        Out1C.textContent = value1C.toFixed(6);
        var Out2C = document.getElementById('Out2C');
        Out2C.textContent = totalCharge3.toFixed(0).replace(/\B(?=(\d{3})+(?!\d))/g, ",");
        var Out3C = document.getElementById('Out3C');
        Out3C.textContent = netTotal3.toFixed(0).replace(/\B(?=(\d{3})+(?!\d))/g, ",");
        var Out4C = document.getElementById('Out4C');
        Out4C.textContent = FixedCharge3.toFixed(2).replace(/\B(?=(\d{3})+(?!\d))/g, ",");
        var Out5C = document.getElementById('Out5C');
        Out5C.textContent = total2Charge3.toFixed(2).replace(/\B(?=(\d{3})+(?!\d))/g, ",");
        var Out6C = document.getElementById('Out6C');
        Out6C.textContent = net2Total3.toFixed(2).replace(/\B(?=(\d{3})+(?!\d))/g, ",");
     
        var Out1D = document.getElementById('Out1D');
        Out1D.textContent = value1D.toFixed(6);
        var Out2D = document.getElementById('Out2D');
        Out2D.textContent = totalCharge4.toFixed(2);
        var Out3D = document.getElementById('Out3D');
        Out3D.textContent = netTotal4.toFixed(2).replace(/\B(?=(\d{3})+(?!\d))/g, ",");
        var Out4D = document.getElementById('Out4D');
        Out4D.textContent = FixedCharge4.toFixed(2).replace(/\B(?=(\d{3})+(?!\d))/g, ",");
        var Out5D = document.getElementById('Out5D');
        Out5D.textContent = total2Charge4.toFixed(2).replace(/\B(?=(\d{3})+(?!\d))/g, ",");
        var Out6D = document.getElementById('Out6D');
        Out6D.textContent = net2Total4.toFixed(2).replace(/\B(?=(\d{3})+(?!\d))/g, ",");
     
     //Out1D.textContent = value1D.toFixed(2).replace(/\B(?=(\d{3})+(?!\d))/g, ",");
