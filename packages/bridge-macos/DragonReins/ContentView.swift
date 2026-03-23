//
//  ContentView.swift
//  DragonReins
//
//  Created by Jamey McElveen on 3/20/26.
//

import SwiftUI
import SystemExtensions

struct ContentView: View {
    @State private var status = "Ready to Rein"

    var body: some View {
        VStack(spacing: 20) {
            Text("🐅 DragonReins").font(.largeTitle)
            Text(status)
            Button("Install Driver") {
                activateDriver()
            }
        }.padding().frame(width: 300, height: 200)
    }

    func activateDriver() {
        let request = OSSystemExtensionRequest.activationRequest(
            forExtensionWithIdentifier: "us.mcelveen.DragonReins.Driver",
            queue: .main
        )
        OSSystemExtensionManager.shared.submitRequest(request)
        status = "Check System Settings -> Privacy"
    }
}
